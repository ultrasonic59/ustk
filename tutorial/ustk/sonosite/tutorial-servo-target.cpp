//! \example tutorial-servo-target.cpp
//! [capture-multi-threaded declaration]
#include <iostream>

#include <visp3/core/vpImageConvert.h>
#include <visp3/core/vpMutex.h>
#include <visp3/core/vpThread.h>
#include <visp3/core/vpTime.h>
#include <visp3/gui/vpDisplayX.h>
#include <visp3/gui/vpPlot.h>
#include <visp3/robot/vpRobotViper850.h>
#include <visp3/sensor/vpV4l2Grabber.h>

#include <visp3/ustk_core/usImagePostScan2D.h>
#include <visp3/ustk_core/usPixelMeterConversion.h>

#include <visp3/ustk_template_tracking/usDenseTracker2D.h>

#if defined(VISP_HAVE_V4L2) && defined(VISP_HAVE_PTHREAD) && defined(VISP_HAVE_VIPER850)

// Shared vars
typedef enum { capture_waiting, capture_started, capture_stopped } t_CaptureState;
t_CaptureState s_capture_state = capture_waiting;
vpImage<unsigned char> s_frame;
vpMutex s_mutex_capture;

bool s_robotIsRunning;
vpMutex s_mutex_control_velocity;
vpColVector s_controlVelocity(6, 0.);

//! [capture-multi-threaded declaration]

//! [capture-multi-threaded captureFunction]
vpThread::Return captureFunction(vpThread::Args args)
{
  vpV4l2Grabber cap = *((vpV4l2Grabber *)args);
  vpImage<unsigned char> frame_;
  bool stop_capture_ = false;

  cap.open(frame_);

  vpRect roi(vpImagePoint(55, 70), vpImagePoint(410, 555)); // roi to remove sonosite banners

  while (!stop_capture_) {
    // Capture in progress
    cap.acquire(frame_, roi); // get a new frame from camera

    // Update shared data
    {
      vpMutex::vpScopedLock lock(s_mutex_capture);
      if (s_capture_state == capture_stopped)
        stop_capture_ = true;
      else
        s_capture_state = capture_started;
      s_frame = frame_;
    }
  }

  {
    vpMutex::vpScopedLock lock(s_mutex_capture);
    s_capture_state = capture_stopped;
  }
  std::cout << "End of capture thread" << std::endl;
  return 0;
}
//! [capture-multi-threaded captureFunction]

//! [capture-multi-threaded displayFunction]
vpThread::Return displayFunction(vpThread::Args args)
{
  (void)args; // Avoid warning: unused parameter args
  usImagePostScan2D<unsigned char> postScan_;
  usDenseTracker2D tracker;
  vpRectOriented rectangle;

  postScan_.setTransducerRadius(0.060);
  postScan_.setTransducerConvexity(true);
  postScan_.setScanLineNumber(128);
  postScan_.setFieldOfView(vpMath::rad(57.0)); // field of view is 57 deg
  postScan_.setDepth(0.12);
  postScan_.setProbeName("Sonosite C60");

  // convert(const usImagePostScan2D<unsigned char> &image, const double &u, const double &v, double &x,  double &y);

  t_CaptureState capture_state_;
  bool display_initialized_ = false;

#if defined(VISP_HAVE_X11)
  vpDisplayX *dpost_scan_ = NULL; // display post-scan image

#endif

  vpPlot plot(1);
  plot.initGraph(0, 1);
  plot.initRange(0, 0, 10, -0.01, 0.01);
  plot.setTitle(0, "X target error");
  plot.setUnitY(0, "error(meters)");
  plot.setLegend(0, 0, "time");

  double xtarget;
  double ytarget;

  bool firstLoopCycle = true;

  double startTime = vpTime::measureTimeMs();

  do {
    s_mutex_capture.lock();
    capture_state_ = s_capture_state;
    s_mutex_capture.unlock();

    // Check if a frame is available
    if (capture_state_ == capture_started) {
      // Create a copy of the captured frame
      {
        vpMutex::vpScopedLock lock(s_mutex_capture);
        postScan_.setData(s_frame);
        postScan_.setHeightResolution(
            (postScan_.getDepth() + postScan_.getTransducerRadius() * (1 - cos(postScan_.getFieldOfView() / 2.0))) /
            postScan_.getHeight());
        postScan_.setWidthResolution(
            (postScan_.getDepth() + postScan_.getTransducerRadius() * (1 - cos(postScan_.getFieldOfView() / 2.0))) /
            postScan_.getHeight());
      }

      // Convert post-scan to pre-scan image
      if (firstLoopCycle) {
        rectangle.setCenter(vpImagePoint(postScan_.getHeight() / 2, postScan_.getWidth() / 2));
        rectangle.setSize(50, 30);
        rectangle.setOrientation(0);
        tracker.init(postScan_, rectangle);
      }

      tracker.update(postScan_);
      rectangle = tracker.getTarget();

      usPixelMeterConversion::convert(postScan_, rectangle.getCenter().get_j(), rectangle.getCenter().get_i(), xtarget,
                                      ytarget);

      std::cout << "Height resolution : " << postScan_.getHeightResolution() << std::endl;
      std::cout << "Width resolution : " << postScan_.getWidthResolution() << std::endl;
      std::cout << "Center i = " << rectangle.getCenter().get_i() << ", j = " << rectangle.getCenter().get_j()
                << std::endl;
      std::cout << "xtarget = " << xtarget << ", ytarget = " << ytarget << std::endl;

      double time = (vpTime::measureTimeMs() - startTime) / 1000.0;

      plot.plot(0, 0, time, xtarget);

      {
        vpMutex::vpScopedLock lock(s_mutex_capture);
        s_controlVelocity = 0.0;
        s_controlVelocity[1] = -1 * xtarget;
      }
      // Check if we need to initialize the display with the first frame
      if (!display_initialized_) {
// Initialize the display
#if defined(VISP_HAVE_X11)
        unsigned int xpos = 10;
        dpost_scan_ = new vpDisplayX(postScan_, xpos, 10, "post-scan");
        display_initialized_ = true;
#endif
      }

      // Display the image
      vpDisplay::display(postScan_);

      vpDisplay::displayLine(postScan_, static_cast<int>(rectangle.getTopLeft().get_i()),
                             static_cast<int>(rectangle.getTopLeft().get_j()),
                             static_cast<int>(rectangle.getBottomLeft().get_i()),
                             static_cast<int>(rectangle.getBottomLeft().get_j()), vpColor::red);
      vpDisplay::displayLine(postScan_, static_cast<int>(rectangle.getBottomLeft().get_i()),
                             static_cast<int>(rectangle.getBottomLeft().get_j()),
                             static_cast<int>(rectangle.getBottomRight().get_i()),
                             static_cast<int>(rectangle.getBottomRight().get_j()), vpColor::red);
      vpDisplay::displayLine(postScan_, static_cast<int>(rectangle.getBottomRight().get_i()),
                             static_cast<int>(rectangle.getBottomRight().get_j()),
                             static_cast<int>(rectangle.getTopRight().get_i()),
                             static_cast<int>(rectangle.getTopRight().get_j()), vpColor::red);
      vpDisplay::displayLine(postScan_, static_cast<int>(rectangle.getTopRight().get_i()),
                             static_cast<int>(rectangle.getTopRight().get_j()),
                             static_cast<int>(rectangle.getTopLeft().get_i()),
                             static_cast<int>(rectangle.getTopLeft().get_j()), vpColor::red);

      // Trigger end of acquisition with a mouse click
      vpDisplay::displayText(postScan_, 10, 10, "Click to exit...", vpColor::red);
      if (vpDisplay::getClick(postScan_, false)) {
        vpMutex::vpScopedLock lock(s_mutex_capture);
        s_capture_state = capture_stopped;
      }

      // Update the display
      vpDisplay::flush(postScan_);
      firstLoopCycle = false;
    } else {
      vpTime::wait(2); // Sleep 2ms
    }
  } while (capture_state_ != capture_stopped);

#if defined(VISP_HAVE_X11)
  delete dpost_scan_;
#endif

  std::cout << "End of display thread" << std::endl;
  return 0;
}
//! [capture-multi-threaded displayFunction]

vpThread::Return controlFunction(vpThread::Args args)
{
  (void)args;
  vpRobotViper850 robot;

  vpMatrix eJe; // robot jacobian

  // Transformation from end-effector frame to the force/torque sensor
  // Note that the end-effector frame is located on the lower part of
  // male component of the tool changer.
  vpHomogeneousMatrix eMs;
  eMs[2][3] = -0.062; // tz = -6.2cm

  // Transformation from force/torque sensor to the probe frame from where
  // we want to control the robot
  vpHomogeneousMatrix sMp;

  // Transformation from force/torque sensor to the end-effector frame
  vpHomogeneousMatrix sMe;
  eMs.inverse(sMe);

  // Build the transformation that allows to convert a velocity in the
  // end-effector frame to the FT sensor frame
  vpVelocityTwistMatrix sVe;
  sVe.buildFrom(sMe);

  vpColVector sHs(6);      // force/torque sensor measures
  vpColVector sHs_star(6); // force/torque sensor desired values in sensor frame
  vpColVector pHp_star(6); // force/torque sensor desired values in probe frame
  vpColVector gHg(6);      // force/torque due to the gravity
  vpMatrix lambda(6, 6);
  // Position of the cog of the object attached after the sensor in the sensor frame
  vpTranslationVector stg;
  vpColVector sHs_bias(6); // force/torque sensor measures for bias

  // Cartesian velocities corresponding to the force/torque control in the
  // sensor frame
  vpColVector v_s(6);
  // Joint velocities corresponding to the force/torque control
  vpColVector q_dot(6);

  // Initialized the force gain
  lambda = 0;
  for (int i = 0; i < 3; i++)
    lambda[i][i] = 0.02 / 6;
  // Initialized the torque gain
  for (int i = 3; i < 6; i++)
    lambda[i][i] = 1. / 2;

  // Initialize the desired force/torque values
  pHp_star = 0;
  pHp_star[2] = 3; // Fz = 3N
  //
  // Case of the C65 US probe
  //
  // Set the probe frame control
  sMp[2][3] = 0.262; // tz = 26.2cm

  // Init the force/torque due to the gravity
  gHg[2] = -(0.696 + 0.476) * 9.81; // m*g

  // Position of the cog of the object attached after the sensor in the sensor frame
  stg[0] = 0;
  stg[1] = 0;
  stg[2] = 0.088; // tz = 88.4mm

  vpRotationMatrix sRp;
  sMp.extract(sRp);
  vpTranslationVector stp;
  sMp.extract(stp);

  vpHomogeneousMatrix eMp = eMs * sMp;
  vpVelocityTwistMatrix eVp(eMp);

  // Get the position of the end-effector in the reference frame
  vpColVector q;
  vpHomogeneousMatrix fMe;
  vpHomogeneousMatrix fMs;
  vpRotationMatrix sRf;
  robot.getPosition(vpRobot::ARTICULAR_FRAME, q);
  robot.get_fMe(q, fMe);
  // Compute the position of the sensor frame in the reference frame
  fMs = fMe * eMs;
  vpHomogeneousMatrix sMf;
  fMs.inverse(sMf);
  sMf.extract(sRf);

  // Build the transformation that allows to convert the forces due to the
  // gravity in the sensor frame
  vpForceTwistMatrix sFg(sMf); // Only the rotation part is to consider
  // Modify the translational part
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      sFg[i + 3][j] = (stg.skew() * sRf)[i][j];

  // Build the transformation that allows to convert a FT expressed in the
  // FT probe frame into the sensor frame
  vpForceTwistMatrix sFp(sMp);

  // Bias the force/torque sensor
  std::cout << "\nBias the force/torque sensor...\n " << std::endl;
  robot.biasForceTorqueSensor();

  // Set the robot to velocity control
  robot.setRobotState(vpRobot::STATE_VELOCITY_CONTROL);

  int iter = 0;
  t_CaptureState capture_state_;

  std::cout << "Starting control loop..." << std::endl;
  do {
    s_mutex_capture.lock();
    capture_state_ = s_capture_state;
    s_mutex_capture.unlock();

    // Check if a frame is available
    if (capture_state_ == capture_started) {

      // Get the force/torque measures from the sensor
      sHs = robot.getForceTorque();

      // Multiply the measures by -1 to get the force/torque exerced by the
      // robot to the environment.
      sHs *= -1;

      // Update the gravity transformation matrix
      robot.getPosition(vpRobot::ARTICULAR_FRAME, q);
      robot.get_fMe(q, fMe);
      // Compute the position of the sensor frame in the reference frame
      fMs = fMe * eMs;
      // Compute the inverse transformation
      fMs.inverse(sMf);
      sMf.extract(sRf);
      // Update the transformation that allows to convert the forces due to the
      // gravity in the sensor frame
      sFg.buildFrom(sMf); // Only the rotation part is to consider
      // Modify the translational part
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
          sFg[i + 3][j] = (stg.skew() * sRf)[i][j];

      if (iter == 0) {
        sHs_bias = sHs - sFg * gHg;
      }

      // Compute rotation in probe frame from control velocity deduced of the confidence map barycenter
      vpColVector v_p;
      {
        vpMutex::vpScopedLock lock(s_mutex_capture);
        v_p = s_controlVelocity;
      }

      v_p[0] = 0;
      v_p[2] = 0;
      v_p[3] = 0;
      v_p[4] = 0;
      v_p[5] = 0;

      // Compute the force/torque control law in the sensor frame
      v_s = lambda * (sFp * pHp_star - (sHs - sFg * gHg - sHs_bias));

      v_s[0] = 0.0;
      v_s[1] = 0.0;
      v_s[3] = 0.0;
      v_s[4] = 0.0;
      v_s[5] = 0.0;

      vpVelocityTwistMatrix eVs;
      sVe.inverse(eVs);

      vpColVector v_e = eVs * v_s + eVp * v_p;

      // Get the robot jacobian eJe
      robot.get_eJe(eJe);

      // Compute the joint velocities to achieve the force/torque control
      q_dot = eJe.pseudoInverse() * v_e;

      // Send the joint velocities to the robot
      robot.setVelocity(vpRobot::ARTICULAR_FRAME, q_dot);

      iter++;
    }
    vpTime::wait(1); // 5
  } while (capture_state_ != capture_stopped);

  std::cout << "End of control thread" << std::endl;
  return 0;
}

//! [capture-multi-threaded mainFunction]
int main(int argc, const char *argv[])
{
  unsigned int opt_input = 1; // Default value is 1 to mach the material in the lab

  // Command line options
  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "--input")
      opt_input = (unsigned int)atoi(argv[i + 1]);
    else if (std::string(argv[i]) == "--help") {
      std::cout << "Usage: " << argv[0] << " [--input <number>] [--help]" << std::endl;
      return 0;
    }
  }

  // Instantiate the grabber
  vpV4l2Grabber g;

  g.setInput(opt_input);
  g.setScale(1);

  // init
  s_robotIsRunning = false;
  s_controlVelocity = vpColVector(6, 0);

  // Start the threads
  vpThread thread_capture((vpThread::Fn)captureFunction, (vpThread::Args)&g);
  vpThread thread_display((vpThread::Fn)displayFunction);
  vpThread thread_control((vpThread::Fn)controlFunction);

  // Wait until thread ends up
  thread_capture.join();
  thread_display.join();
  thread_control.join();

  return 0;
}
//! [capture-multi-threaded mainFunction]

#else
int main()
{
#ifndef VISP_HAVE_V4L2
  std::cout << "You should enable V4L2 to make this example working..." << std::endl;
#elif !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  std::cout << "You should enable pthread usage and rebuild ViSP..." << std::endl;
#else
  std::cout << "Multi-threading seems not supported on this platform" << std::endl;
#endif
#ifndef VISP_HAVE_VIPER850
  std::cout << "You need viper 850 robot to run this example" << std::endl;
#endif
}

#endif
