#include "VtkViewer.h"

//class vtkResliceCursorCallback : public vtkCommand
//{
//public:
//	static vtkResliceCursorCallback *New()
//	{
//		return new vtkResliceCursorCallback;
//	}
//
//	void Execute(vtkObject *caller, unsigned long ev,
//		void *callData)
//	{
//
//		if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
//			ev == vtkCommand::WindowLevelEvent ||
//			ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
//		{
//			// Render everything
//			for (int i = 0; i < 3; i++)
//			{
//				this->RCW[i]->Render();
//			}
//			this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
//			return;
//		}
//
//		vtkImagePlaneWidget* ipw =
//			dynamic_cast<vtkImagePlaneWidget*>(caller);
//		if (ipw)
//		{
//			double* wl = static_cast<double*>(callData);
//
//			if (ipw == this->IPW[0])
//			{
//				this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
//				this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
//			}
//			else if (ipw == this->IPW[1])
//			{
//				this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
//				this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
//			}
//			else if (ipw == this->IPW[2])
//			{
//				this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
//				this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
//			}
//		}
//
//		vtkResliceCursorWidget *rcw = dynamic_cast<
//			vtkResliceCursorWidget *>(caller);
//		if (rcw)
//		{
//			vtkResliceCursorLineRepresentation *rep = dynamic_cast<
//				vtkResliceCursorLineRepresentation *>(rcw->GetRepresentation());
//			// Although the return value is not used, we keep the get calls
//			// in case they had side-effects
//			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
//			for (int i = 0; i < 3; i++)
//			{
//				vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(
//					this->IPW[i]->GetPolyDataAlgorithm());
//				ps->SetOrigin(this->RCW[i]->GetResliceCursorRepresentation()->
//					GetPlaneSource()->GetOrigin());
//				ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->
//					GetPlaneSource()->GetPoint1());
//				ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->
//					GetPlaneSource()->GetPoint2());
//
//				// If the reslice plane has modified, update it on the 3D widget
//				this->IPW[i]->UpdatePlacement();
//			}
//		}
//
//		// Render everything
//		for (int i = 0; i < 3; i++)
//		{
//			this->RCW[i]->Render();
//		}
//		this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
//	}
//
//	vtkResliceCursorCallback() {}
//	vtkImagePlaneWidget* IPW[3];
//	vtkResliceCursorWidget *RCW[3];
//};

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

	//vtkSmartPointer<vtkResliceCursorCallback> cbk =
	//	vtkSmartPointer<vtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		//cbk->IPW[i] = planeWidget[i];
		//cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
		/*riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::WindowLevelEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(
		 vtkResliceCursorWidget::ResetCursorEvent, cbk);
		riw[i]->GetInteractorStyle()->AddObserver(
		 vtkCommand::WindowLevelEvent, cbk);*/

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