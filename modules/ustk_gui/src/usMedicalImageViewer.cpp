/****************************************************************************
 *
 * This file is part of the ustk software.
 * Copyright (C) 2016 - 2017 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ustk with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at ustk@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors:
 * Marc Pouliquen
 *
 *****************************************************************************/

/**
* @file usMedicalImageViewer.cpp
* @brief Graphical main window containing 4 vtk views.
*/

#include <visp3/ustk_core/usImagePostScan3D.h>
#include <visp3/ustk_gui/usVTKConverter.h>
#include <visp3/ustk_gui/usMedicalImageViewer.h>

#ifdef USTK_HAVE_VTK_QT

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkResliceImageViewer.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorThickLineRepresentation.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkResliceCursor.h>
#include <vtkMetaImageReader.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
#include <vtkPlane.h>
#include <vtkImageData.h>
#include <vtkCommand.h>
#include <vtkPlaneSource.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageSlabReslice.h>
#include <vtkBoundedPlanePointPlacer.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkHandleRepresentation.h>
#include <vtkResliceImageViewerMeasurements.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkCamera.h>
#include <vtkRendererCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkAbstractTransform.h>
#include <vtkImageActor.h>
#include <vtkMapper.h>
#include <vtkActor2D.h>
#include <vtkActor.h>

#include <QDesktopWidget>
#include <QResizeEvent>


//----------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand
{
public:
  static vtkResliceCursorCallback *New()
  { return new vtkResliceCursorCallback; }

  //implementing virtual method of parent class but we don't need arguments, so they've been remove to avoid warings at compilation
  void Execute( vtkObject *, unsigned long ,
                void * )
  {
    // Render everything
    for (int i = 0; i < 3; i++)
    {
      if(i==0) {
        /*this->RIW[i]->GetRenderer()->GetActiveCamera()->Print(std::cout);
        //this->RIW[i]->GetRenderer()->GetActiveCamera()->GetClippingRange()->Print(;)
        double dop[3];
        this->RIW[i]->GetRenderer()->GetActiveCamera()->GetDirectionOfProjection(dop);
        //std::cout << "DOP : " << dop[0] << ","<< dop[1] << ","<< dop[2] << std::endl;
        //std::cout << "ROLL : " <<this->RIW[i]->GetRenderer()->GetActiveCamera()->GetRoll() << std::endl;
        //this->RIW[i]->GetRenderer()->GetActiveCamera()->SetRoll(90);

        //view plane normal
        double vpn[3];
        this->RIW[i]->GetRenderer()->GetActiveCamera()->GetViewPlaneNormal(vpn);
        //std::cout << "VPN : " << vpn[0] << ","<< vpn[1] << ","<< vpn[2] << std::endl;

        //projection transform
        vtkMatrix4x4* pt;
        pt = this->RIW[i]->GetRenderer()->GetActiveCamera()->GetProjectionTransformMatrix(this->RIW[i]->GetRenderer());
        //std::cout << "proj transform : " << std::endl;
        for(int a = 0 ; a < 4 ; a++) {
          for(int b=0;b<4;b++) {
            std::cout << pt->Element[a][b] << "\t\t";
          }
          std::cout << std::endl;
        }

        this->RIW[i]->GetImageActor()->Print(std::cout);

        double bounds[6];
    this->RIW[i]->GetImageActor()->GetDisplayBounds(bounds);
    std::cout << "extend["<<i<<"] : " << bounds[0] << "," << bounds[1] << "," << bounds[2] << "," << bounds[3] << "," << bounds[4] << "," << bounds[5] << std::endl;
        int extend[6];
    this->RIW[i]->GetImageActor()->GetDisplayExtent(extend);
    std::cout << "extend["<<i<<"] : " << extend[0] << "," << extend[1] << "," << extend[2] << "," << extend[3] << "," << extend[4] << "," << extend[5] << std::endl;
*/
      }
      //if(i==0)
        this->RIW[i]->Render();

    }


    //widget3D->getPlane1()->Get;
    //std::cout << "Plane 2 " << std::endl;
    //widget3D->getPlane2()->Print(std::cout);
    //std::cout << "Plane 3 " << std::endl;
    //widget3D->getPlane3()->Print(std::cout);
    widget3D->update();
    //std::cout << "3D view Plane 1 " << std::endl;
    //double norm[3];
    //widget3D->getPlane1()->GetNormal(norm);
    //std::cout << "NORMAL : " << norm[0] << ","<< norm[1] << ","<< norm[2] << std::endl;
  }

  vtkResliceCursorCallback() {}
  us3DSceneWidget* widget3D;
  //vtkResliceCursorWidget *RCW[3];
  usResliceImageViewer* RIW[3];
};

/**
* Constructor.
* @param imageFileName the mhd file to read.
*/
usMedicalImageViewer::usMedicalImageViewer(std::string imageFileName )
{
  this->setupUi();
  usImageIo::read(postScanImage,imageFileName);
  usVTKConverter::convert(postScanImage,vtkImage);

  int imageDims[3];
  double spacing[3];
  vtkImage->GetDimensions(imageDims);
  vtkImage->GetSpacing(spacing);

  std::cout << "convert done " << std::endl;

  for (int i = 0; i < 3; i++)
  {
    riw[i] = vtkSmartPointer< usResliceImageViewer >::New();
  }

  this->view1->SetRenderWindow(riw[0]->GetRenderWindow());
  riw[0]->SetupInteractor(
        this->view1->GetRenderWindow()->GetInteractor());

  this->view2->SetRenderWindow(riw[1]->GetRenderWindow());
  riw[1]->SetupInteractor(
        this->view2->GetRenderWindow()->GetInteractor());

  this->view3->SetRenderWindow(riw[2]->GetRenderWindow());
  riw[2]->SetupInteractor(
        this->view3->GetRenderWindow()->GetInteractor());

  for (int i = 0; i < 3; i++)
  {
    // make them all share the same reslice cursor object.
    vtkResliceCursorLineRepresentation *rep =
        vtkResliceCursorLineRepresentation::SafeDownCast(
          riw[i]->GetResliceCursorWidget()->GetRepresentation());
    riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

    rep->GetResliceCursorActor()->
        GetCursorAlgorithm()->SetReslicePlaneNormal(i);

    riw[i]->SetInputData(vtkImage);
    riw[i]->SetSliceOrientation(i);
    riw[i]->SetSlice(imageDims[i]/2);
    //riw[i]->GetRenderer()->GetActiveCamera()->SetRoll(180);
    riw[i]->SetResliceModeToOblique();
    riw[i]->SliceScrollOnMouseWheelOff(); // scroll on mouse wheel not working weell (differents views not synchronized), so we disable it
  }
/*
  plane1 = vtkPlane::New();
  plane1->SetOrigin(0,0,0);
  plane1->SetNormal(1,0,0);

  plane2 = vtkPlane::New();
  plane2->SetNormal(0,1,0);
  plane2->SetOrigin(0,0,0);

  plane3 = vtkPlane::New();
  plane3->SetNormal(0,0,1);
  plane3->SetOrigin(0,0,0);*/

  this->view4->setImageData(vtkImage);
  this->view4->setPlanes(riw[0]->GetResliceCursor()->GetPlane(0),riw[1]->GetResliceCursor()->GetPlane(1),riw[2]->GetResliceCursor()->GetPlane(2));
  //this->view4->setPlanes(plane1,plane2,plane3);

  this->view4->init();


  //////////////////////////////////////////////////////////////////////////////
  /*//VIEW 1
  imageMapper1 = vtkSmartPointer<vtkImageResliceMapper>::New();
  std::cout << "toto" << std::endl;
  imageMapper1->SetInputData(vtkImage);
  std::cout << "toto" << std::endl;
  imageMapper1->SetSlicePlane(plane1);
  std::cout << "toto" << std::endl;
  //imageMapper1->Update();
  imageMapper1->SliceFacesCameraOn();
  std::cout << "toto" << std::endl;

  vtkSmartPointer<vtkImageSlice> slice1 = vtkSmartPointer<vtkImageSlice>::New();
  slice1->SetMapper(imageMapper1);

  vtkSmartPointer<vtkRenderer> renderer1 = vtkSmartPointer<vtkRenderer>::New();
  renderer1->AddViewProp(slice1);
  renderer1->ResetCamera();

  view1->GetRenderWindow()->AddRenderer(renderer1);

  std::cout << "toto" << std::endl;
  std::cout << "toto" << std::endl;
  //interaction
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
  renderWindowInteractor->SetInteractorStyle(style);
  view1->GetRenderWindow()->SetInteractor(renderWindowInteractor);



  std::cout << "toto" << std::endl;



  vtkSmartPointer<vtkCamera> camera1 = renderer1->GetActiveCamera();
  std::cout << "cam1 @ : " << (void*)camera1 << std::endl;
  camera1->SetViewUp (0, 0, -1);
  camera1->SetPosition (1, 0, 0);
  camera1->SetFocalPoint (0, 0, 0);
  camera1->ComputeViewPlaneNormal();
  camera1->Azimuth(30.0);
  camera1->Elevation(30.0);

  //verify the camera plane
  double plane[3];
  camera1->GetViewPlaneNormal(plane);
  std::cout << "plane 1 : " << *plane << ", " << plane[1] << ", " << plane[2] << std::endl;

  view1->GetRenderWindow()->AddRenderer(renderer1);



  //VIEW 2
  vtkSmartPointer<vtkRenderer> renderer2 = vtkSmartPointer<vtkRenderer>::New();
  //vtkSmartPointer<vtkMapper> mapper2 = vtkSmartPointer<vtkRenderer>::New();
  //vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkRenderer>::New();
  //mapper2 = view4->getMapper2();
  //actor2->SetMapper(view4->getMapper2());
  //renderer2->AddActor(actor2);


  vtkSmartPointer<vtkImageMapper> imageMapper = vtkSmartPointer<vtkImageMapper>::New();


  renderer2->AddActor(view4->getActor2());

  vtkSmartPointer<vtkCamera> camera2 = renderer2->GetActiveCamera();
  std::cout << "cam2 @ : " << (void*)camera2 << std::endl;
  camera2->SetViewUp (0, 0, -1);
  camera2->SetPosition (0, 1, 0);
  camera2->SetFocalPoint (0, 0, 0);
  camera2->ComputeViewPlaneNormal();
  camera2->Azimuth(30.0);
  camera2->Elevation(30.0);

  //verify the camera plane
  camera2->GetViewPlaneNormal(plane);
  std::cout << "plane 2 : " << *plane << ", " << plane[1] << ", " << plane[2] << std::endl;

  view2->GetRenderWindow()->AddRenderer(renderer2);



  //VIEW 3
  vtkSmartPointer<vtkRenderer> renderer3 = vtkSmartPointer<vtkRenderer>::New();
  renderer3->AddActor(view4->getActor3());
  vtkSmartPointer<vtkCamera> camera3 = renderer3->GetActiveCamera();
  std::cout << "cam3 @ : " << (void*)camera3 << std::endl;
  camera3->SetViewUp (0, 0, -1);
  camera3->SetPosition (0, 1, 0);
  camera3->SetFocalPoint (0, 0, 0);
  camera3->ComputeViewPlaneNormal();
  camera3->Azimuth(30.0);
  camera3->Elevation(30.0);

  //verify the camera plane
  camera3->GetViewPlaneNormal(plane);
  std::cout << "plane 3 : " << *plane << ", " << plane[1] << ", " << plane[2] << std::endl;

  view3->GetRenderWindow()->AddRenderer(renderer3);
*/

  //////////////////////////////////////////////////////////////////////////////

  vtkSmartPointer<vtkResliceCursorCallback> cbk =
      vtkSmartPointer<vtkResliceCursorCallback>::New();

  cbk->widget3D = this->view4;

  for (int i = 0; i < 3; i++)
  {
    //cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
    cbk->RIW[i] = riw[i];
    riw[i]->GetResliceCursorWidget()->AddObserver(
          vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk );
    riw[i]->GetResliceCursorWidget()->AddObserver(
          vtkResliceCursorWidget::WindowLevelEvent, cbk );
    riw[i]->GetResliceCursorWidget()->AddObserver(
          vtkResliceCursorWidget::ResetCursorEvent, cbk );
  }


  //riw[0]->GetImageActor()->SetMapper(view4->getMapper1());
  //riw[1]->GetImageActor()->SetMapper(view4->getMapper2());
  //riw[2]->GetImageActor()->SetMapper(view4->getMapper3());

  for (int i = 0; i < 3; i++)
  {
    riw[i]->SetResliceMode(1);
    riw[i]->GetRenderer()->ResetCamera();
    if(i==2)
      riw[i]->GetRenderer()->GetActiveCamera()->SetRoll(180);
    riw[i]->Render();
  }

  this->view4->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetRoll(180);
  // Set up action signals and slots
  //connect(this->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
  connect(this->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
  //connect(this->resetColorsButton, SIGNAL(pressed()), this, SLOT(ResetColorMap()));
  //connect(this->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));
  ResetViews();
}

/**
* Exit slot, to exit the QApplication.
*/
void usMedicalImageViewer::slotExit()
{
  qApp->exit();
}

/**
* Reset views slot : reset the planes positions at the middle of the volume.
*/
void usMedicalImageViewer::ResetViews()
{
  // Reset the reslice image views
  for (int i = 0; i < 3; i++)
  {
    /*riw[i]->Reset();
    riw[i]->SetResliceMode(1);
    vi[i]->GetRenderer()->ResetCamera();
    if(i==2)
      riw[i]->GetRenderer()->GetActiveCamera()->SetRoll(180);*/
  }
  /*view1->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
  view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
  view3->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();*/
  this->Render();
}

/**
* Reset color map slot : reset the color map to initial one in each view.
*/
void usMedicalImageViewer::ResetColorMap()
{
  // Render everything
  for (int i = 0; i < 3; i++)
  {
    riw[i]->GetLookupTable()->SetRange(0,255);
    riw[i]->GetResliceCursorWidget()->Render();
  }
}

/**
* Render slot, to recompute all the views.
*/
void usMedicalImageViewer::Render()
{
  for (int i = 0; i < 3; i++)
  {
    //riw[i]->Render();
  }

  this->view1->GetRenderWindow()->Render();
  this->view2->GetRenderWindow()->Render();
  this->view3->GetRenderWindow()->Render();

  this->view1->update();
  this->view2->update();
  this->view3->update();
  this->view4->update();
}

/**
* Slot to add a distance measure widget to wiew 1.
*/
void usMedicalImageViewer::AddDistanceMeasurementToView1()
{
  this->AddDistanceMeasurementToView(1);
}

/**
* Slot to add a distance measure widget to a wiew.
* @param i View number to add the measure on.
*/
void usMedicalImageViewer::AddDistanceMeasurementToView(int i)
{
  // remove existing widgets.
  if (this->DistanceWidget[i])
  {
    this->DistanceWidget[i]->SetEnabled(0);
    this->DistanceWidget[i] = NULL;
  }

  // add new widget
  this->DistanceWidget[i] = vtkSmartPointer< vtkDistanceWidget >::New();
  this->DistanceWidget[i]->SetInteractor(
        this->riw[i]->GetResliceCursorWidget()->GetInteractor());

  // Set a priority higher than our reslice cursor widget
  this->DistanceWidget[i]->SetPriority(
        this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

  vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep =
      vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
  vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep =
      vtkSmartPointer< vtkDistanceRepresentation2D >::New();
  distanceRep->SetHandleRepresentation(handleRep);
  this->DistanceWidget[i]->SetRepresentation(distanceRep);
  distanceRep->InstantiateHandleRepresentation();
  distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
  distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

  // Add the distance to the list of widgets whose visibility is managed based
  // on the reslice plane by the ResliceImageViewerMeasurements class
  this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

  this->DistanceWidget[i]->CreateDefaultRepresentation();
  this->DistanceWidget[i]->EnabledOn();
}

/**
* Setup all the widgets in the window.
*/
void usMedicalImageViewer::setupUi() {
  this->setMinimumSize(640,480);
  QRect screenRect = QApplication::desktop()->screenGeometry();
  this->resize(screenRect.size());

  gridLayoutWidget = new QWidget(this);
  gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
  gridLayoutWidget->setGeometry(QRect(10, 10, screenRect.width() - 200, screenRect.height() - 40));
  gridLayout_2 = new QGridLayout(gridLayoutWidget);
  gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
  gridLayout_2->setContentsMargins(0, 0, 0, 0);
  view2 = new usViewerWidget(gridLayoutWidget);
  view2->setObjectName(QString::fromUtf8("view2"));

  gridLayout_2->addWidget(view2, 1, 0, 1, 1);

  view4 = new us3DSceneWidget(gridLayoutWidget);
  view4->setObjectName(QString::fromUtf8("view4"));

  gridLayout_2->addWidget(view4, 0, 1, 1, 1);

  view3 = new usViewerWidget(gridLayoutWidget);
  view3->setObjectName(QString::fromUtf8("view3"));

  gridLayout_2->addWidget(view3, 1, 1, 1, 1);

  view1 = new usViewerWidget(gridLayoutWidget);
  view1->setObjectName(QString::fromUtf8("view1"));

  gridLayout_2->addWidget(view1, 0, 0, 1, 1);

  resetButton = new QPushButton(this);
  resetButton->setObjectName(QString::fromUtf8("resetButton"));
  resetButton->setText(QString::fromUtf8("Reset views"));
  resetButton->setGeometry(QRect(screenRect.width() - 180, 30, 160, 31));

  resetColorsButton = new QPushButton(this);
  resetColorsButton->setObjectName(QString::fromUtf8("resetColorsButton"));
  resetColorsButton->setText(QString::fromUtf8("Reset colormap"));
  resetColorsButton->setGeometry(QRect(screenRect.width() - 180, 80, 160, 31));

  AddDistance1Button = new QPushButton(this);
  AddDistance1Button->setObjectName(QString::fromUtf8("AddDistance1Button"));
  AddDistance1Button->setText(QString::fromUtf8("Add distance 1"));
  AddDistance1Button->setGeometry(QRect(screenRect.width() - 180, 130, 160, 31));

}

/**
* Get the resize event of the window, to re-comute size and positions of all widgets/layouts.
*/
void usMedicalImageViewer::resizeEvent(QResizeEvent* event)
{
  //Min size : 640*480
  if(event->size().width() >= 640 && event->size().height() >= 480) {
    QMainWindow::resizeEvent(event);
    gridLayoutWidget->setGeometry(QRect(10, 10, event->size().width() - 220, event->size().height() - 20));
    resetButton->setGeometry(QRect(event->size().width() - 180, 30, 160, 31));
    resetColorsButton->setGeometry(QRect(event->size().width() - 180, 80, 160, 31));
    AddDistance1Button->setGeometry(QRect(event->size().width() - 180, 130, 160, 31));
  }
}
#endif
