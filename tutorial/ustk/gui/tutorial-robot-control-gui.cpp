//! \example tutorial-robot-control-gui.cpp

#include <iostream>
#include <visp3/ustk_core/usConfig.h>

#if defined(VISP_HAVE_MODULE_USTK_GUI) && defined(VISP_HAVE_MODULE_USTK_GRABBER) && defined(VISP_HAVE_VIPER850)

#include <visp3/ustk_grabber/usNetworkGrabberPreScan2D.h>
#include <visp3/ustk_gui/usConfidenceMapController.h>
#include <visp3/ustk_gui/usImageDisplayWidgetRobotControl.h>
#include <visp3/ustk_gui/usRobotManualControlWidget.h>
#include <visp3/ustk_gui/usUltrasonixClientWidget.h>
#include <visp3/ustk_gui/usViper850WrapperVelocityControl.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>

int main(int argc, char **argv)
{

  QApplication app(argc, argv);
  app.setApplicationName(QString("USTK display widget"));

  // Qt widgets
  usImageDisplayWidgetRobotControl *widget = new usImageDisplayWidgetRobotControl();
  widget->enableControlArrows();
  widget->enableFeaturesDisplay();

  usRobotManualControlWidget *robotControlPanel = new usRobotManualControlWidget();
  usUltrasonixClientWidget *ultrasonixControlWidet = new usUltrasonixClientWidget();

  QWidget *centralWidget = new QWidget();
  QHBoxLayout *mainLayout = new QHBoxLayout();
  mainLayout->addWidget(ultrasonixControlWidet);
  mainLayout->addWidget(widget, 2);
  mainLayout->addWidget(robotControlPanel);
  centralWidget->setLayout(mainLayout);

  // robot control
  QThread *threadRobotControl = new QThread();
  usViper850WrapperVelocityControl viperControl;
  viperControl.moveToThread(threadRobotControl);
  threadRobotControl->start();
  viperControl.run();

  // confidence-map-based controller
  usConfidenceMapController *confidenceController = new usConfidenceMapController();
  QThread *confidenceThread = new QThread();
  confidenceController->moveToThread(confidenceThread);
  confidenceThread->start();

  // grabber
  QThread *grabbingThread = new QThread();
  usNetworkGrabberPreScan2D *qtGrabber = new usNetworkGrabberPreScan2D();
  qtGrabber->moveToThread(grabbingThread);
  grabbingThread->start();

  QMainWindow window;
  window.setCentralWidget(centralWidget);
  window.resize(1280, 480);
  window.show();

  // sliders robot controls
  QObject::connect(robotControlPanel, SIGNAL(changeVX(int)), &viperControl, SLOT(setXVelocity(int)));
  QObject::connect(robotControlPanel, SIGNAL(changeVY(int)), &viperControl, SLOT(setYVelocity(int)));
  QObject::connect(robotControlPanel, SIGNAL(changeVZ(int)), &viperControl, SLOT(setZVelocity(int)));
  QObject::connect(robotControlPanel, SIGNAL(changeWX(int)), &viperControl, SLOT(setXAngularVelocity(int)));
  QObject::connect(robotControlPanel, SIGNAL(changeWY(int)), &viperControl, SLOT(setYAngularVelocity(int)));
  QObject::connect(robotControlPanel, SIGNAL(changeWZ(int)), &viperControl, SLOT(setZAngularVelocity(int)));

  // buttons robot controls
  QObject::connect(widget, SIGNAL(moveLeft()), &viperControl, SLOT(moveLeft()));
  QObject::connect(widget, SIGNAL(moveRight()), &viperControl, SLOT(moveRight()));
  QObject::connect(widget, SIGNAL(stopMove()), &viperControl, SLOT(stopMoveLateral()));

  QObject::connect(robotControlPanel, SIGNAL(initRobot()), &viperControl, SLOT(init()));
  QObject::connect(robotControlPanel, SIGNAL(startRobot()), &viperControl, SLOT(run()));
  QObject::connect(robotControlPanel, SIGNAL(stopRobot()), &viperControl, SLOT(stop()));

  QObject::connect(robotControlPanel, SIGNAL(activateAutomaticForceControl()), &viperControl,
                   SLOT(startAutomaticForceControl()));
  QObject::connect(robotControlPanel, SIGNAL(disableAutomaticForceControl()), &viperControl,
                   SLOT(stopAutomaticForceControl()));

  // manage errors
  QObject::connect(&viperControl, SIGNAL(robotError()), robotControlPanel, SLOT(robotErrorSlot()));

  // grabber control
  qRegisterMetaType<QHostAddress>("QHostAddress");
  qRegisterMetaType<usNetworkGrabber::usInitHeaderSent>("usNetworkGrabber::usInitHeaderSent");
  QObject::connect(ultrasonixControlWidet, SIGNAL(connectToServer(QHostAddress)), qtGrabber,
                   SLOT(connectToServer(QHostAddress)));
  QObject::connect(ultrasonixControlWidet, SIGNAL(initAcquisition(usNetworkGrabber::usInitHeaderSent)), qtGrabber,
                   SLOT(initAcquisitionSlot(usNetworkGrabber::usInitHeaderSent)));
  QObject::connect(ultrasonixControlWidet, SIGNAL(center3DProbeMotor()), qtGrabber, SLOT(center3DProbeMotor()));
  QObject::connect(ultrasonixControlWidet, SIGNAL(runAcquisition()), qtGrabber, SLOT(runAcquisition()));
  QObject::connect(ultrasonixControlWidet, SIGNAL(stopAcquisition()), qtGrabber, SLOT(stopAcquisition()));

  // send new images via qt signal
  qRegisterMetaType<usImagePreScan2D<unsigned char> >("usImagePreScan2D<unsigned char>");
  QObject::connect(qtGrabber, SIGNAL(newFrame(usImagePreScan2D<unsigned char>)), widget,
                   SLOT(updateFrame(usImagePreScan2D<unsigned char>)));

  // confidence controller
  QObject::connect(qtGrabber, SIGNAL(newFrame(usImagePreScan2D<unsigned char>)), confidenceController,
                   SLOT(updateImage(usImagePreScan2D<unsigned char>)));
  QObject::connect(widget, SIGNAL(confidenceServoing(bool)), confidenceController, SLOT(activateController(bool)));
  QObject::connect(confidenceController, SIGNAL(updateProbeOrientation(int)), &viperControl,
                   SLOT(setZAngularVelocity(int)));
  QObject::connect(confidenceController, SIGNAL(confidenceBarycenterAngle(double)), widget,
                   SLOT(updateConfidenceAngle(double)));
  QObject::connect(confidenceController, SIGNAL(confidenceMap(usImagePreScan2D<unsigned char>)), widget,
                   SLOT(updateConfidenceMap(usImagePreScan2D<unsigned char>)));

  app.exec();

  grabbingThread->exit();

  delete widget;
  delete robotControlPanel;
  delete centralWidget;
  delete mainLayout;
  delete grabbingThread;
  delete qtGrabber;

  return 0;
}

#else
int main()
{
  std::cout << "You should build ustk_gui and ustk_grabber, and have a viper850 robot to run this tutorial"
            << std::endl;
  return 0;
}

#endif
