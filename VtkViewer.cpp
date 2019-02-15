#include "VtkViewer.h"

VtkViewer::VtkViewer(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::VtkViewer)
{
	ui->setupUi(this);
}

void VtkViewer::Display(QString imagePath)
{
	std::string dirStr = imagePath.toStdString();
	
	vtkSmartPointer< vtkNIFTIImageReader > reader =
		vtkSmartPointer< vtkNIFTIImageReader >::New();

	vtkSmartPointer<vtkImageData> image = 
		vtkSmartPointer<vtkImageData>::New();

	reader->SetFileName(dirStr.c_str());
	reader->Update();
	image->ShallowCopy(reader->GetOutput());

	this->ConstructViews(image);
	//this->ui->stackedWidget->setCurrentIndex(1);
}

void VtkViewer::ConstructViews(vtkImageData *image)
{
	int imageDims[3];
	image->GetDimensions(imageDims);

	for (int i = 0; i < 3; i++)
	{
		riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
		vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
		riw[i]->SetRenderWindow(renderWindow);
	}

	this->ui->widgetA->SetRenderWindow(riw[0]->GetRenderWindow());
	riw[0]->SetupInteractor(
		this->ui->widgetA->GetRenderWindow()->GetInteractor()
	);

	this->ui->widgetC->SetRenderWindow(riw[1]->GetRenderWindow());
	riw[1]->SetupInteractor(
		this->ui->widgetC->GetRenderWindow()->GetInteractor()
	);

	this->ui->widgetS->SetRenderWindow(riw[2]->GetRenderWindow());
	riw[2]->SetupInteractor(
		this->ui->widgetS->GetRenderWindow()->GetInteractor()
	);

	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation *rep = 
			vtkResliceCursorLineRepresentation::SafeDownCast(
				riw[i]->GetResliceCursorWidget()->GetRepresentation()
			);

		riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

		rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

		riw[i]->SetInputData(image);
		riw[i]->SetSliceOrientation(i);
		riw[i]->SetResliceModeToAxisAligned();
	}

	vtkSmartPointer<vtkCellPicker> picker =
		vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp =
		vtkSmartPointer<vtkProperty>::New();

	vtkSmartPointer< vtkRenderer > ren =
		vtkSmartPointer< vtkRenderer >::New();

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	this->ui->widget3->SetRenderWindow(renderWindow);
	this->ui->widget3->GetRenderWindow()->AddRenderer(ren);
	vtkRenderWindowInteractor *iren = this->ui->widget3->GetInteractor();

	for (int i = 0; i < 3; i++)
	{
		planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
		planeWidget[i]->SetInteractor(iren);
		planeWidget[i]->SetPicker(picker);
		planeWidget[i]->RestrictPlaneToVolumeOn();
		double color[3] = { 0, 0, 0 };
		color[i] = 1;
		planeWidget[i]->GetPlaneProperty()->SetColor(color);

		color[0] /= 4.0;
		color[1] /= 4.0;
		color[2] /= 4.0;
		riw[i]->GetRenderer()->SetBackground(color);

		planeWidget[i]->SetTexturePlaneProperty(ipwProp);
		planeWidget[i]->TextureInterpolateOff();
		planeWidget[i]->SetResliceInterpolateToLinear();
		planeWidget[i]->SetInputData(image);
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		planeWidget[i]->DisplayTextOn();
		planeWidget[i]->SetDefaultRenderer(ren);
		planeWidget[i]->SetWindowLevel(1358, -27);
		planeWidget[i]->On();
		planeWidget[i]->InteractionOn();
	}

	this->Render();

	vtkSmartPointer<vtkResliceCursorCallback> cbk =
		vtkSmartPointer<vtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		cbk->IPW[i] = planeWidget[i];
		cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::WindowLevelEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::ResetCursorEvent, cbk);
		riw[i]->GetInteractorStyle()->AddObserver(
		 vtkCommand::WindowLevelEvent, cbk);

		// Make them all share the same color map.
		riw[i]->SetLookupTable(riw[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
		//planeWidget[i]->GetColorMap()->SetInput(
		//	riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput()
		//);
		planeWidget[i]->SetColorMap(
			riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()
		);
	}

	this->ui->widgetA->show();
	this->ui->widgetC->show();
	this->ui->widgetS->show();
}

void VtkViewer::ResetViews()
{
	// Reset the reslice image views
	for (int i = 0; i < 3; i++)
	{
		riw[i]->Reset();
	}

	// Also sync the Image plane widget on the 3D top right view with any
	// changes to the reslice cursor.
	for (int i = 0; i < 3; i++)
	{
		vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(
			planeWidget[i]->GetPolyDataAlgorithm()
		);
		ps->SetNormal(riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
		ps->SetCenter(riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

		// If the reslice plane has modified, update it on the 3D widget
		this->planeWidget[i]->UpdatePlacement();
	}

	// Render in response to changes.
	this->Render();
}

void VtkViewer::Render()
{
	for (int i = 0; i < 3; i++)
	{
		riw[i]->Render();
	}
	
	this->ui->widgetA->GetRenderWindow()->Render();
}