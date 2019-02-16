#ifndef VTK_VIEWER_H
#define VTK_VIEWER_H

#include "ViewerBase.h"

#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkNIFTIImageReader.h"
#include "vtkDistanceRepresentation.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkDistanceWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include "vtkHandleRepresentation.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageSlabReslice.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLookupTable.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkProperty.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkResliceImageViewer.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursor.h"
#include "vtkResliceImageViewerMeasurements.h"
//#include <itkImageToVTKImageFilter.h>
#include <vtkMetaImageWriter.h>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkImageData.h>

#include "ui_VtkViewer.h"

class VtkViewer : public ViewerBase
{
	Q_OBJECT

public:
	explicit VtkViewer(QWidget *parent = nullptr);

	void Display(QString imagePath);

private:
	//void SetupWidgets();
	void ConstructViews(vtkImageData *image);
	void ResetViews();
	void Render();

	Ui::VtkViewer *ui;
	vtkSmartPointer< vtkResliceImageViewer > riw[3];
	vtkSmartPointer< vtkImagePlaneWidget > planeWidget[3];
};

#endif // ! VTK_VIEWER_H
