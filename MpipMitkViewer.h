#ifndef MPIP_MITK_VIEWER_H
#define MPIP_MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <QDebug>
#include <QMouseEvent>
#include <mitkStandaloneDataStorage.h>
//#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include "QmitkStdMultiWidget.h"

class MpipMitkViewer : public QmitkStdMultiWidget /*public QWidget*/
{
	Q_OBJECT

public:
	MpipMitkViewer();

	void Display(QString imagePath);
	void SetupWidgets();

	void mousePressEvent(QMouseEvent *e) override
	{
		//e->accept();
		qDebug() << QString("MOUSE PRESSED");
		//this->GetRenderWindow1()->mousePressEvent(e);
		/*QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, e->pos(), Qt::RightButton, QFlags<Qt::MouseButton>(Qt::NoButton),
			QFlags<Qt::KeyboardModifier>(Qt::NoModifier));
		QApplication::postEvent(this->GetRenderWindow1(), ev);*/
	}

	//void mousePressEvent(QMouseEvent *e) override
	//{
	//	//switch (e->button())
	//	//{
	//	//case Qt::RightButton:
	//		//if (::GetKeyState(VK_CONTROL) & KEY_DOWN) //right click + ctrl
	//		//{
	//			mitk::Point3D point = getMousePosition(e->pos());
	//			//set the crosshair to the position of the right mouse click
	//			mitk::Geometry3D *geometry = m_DataStorage->GetNode()->GetGeometry();
	//			mitk::Point3D point3D;
	//			geometry->IndexToWorld(point, point3D);
	//			this->MoveCrossToPosition(point3D);
	//			//emit signal to the main class
	//			//emit(rightClickAndCtrl(point));
	//		//}

	//	//	break;
	//	//default:
	//	//	break;
	//	//}
	//}


	//mitk::Point3D getMousePosition(QPoint point)
	//{
	//	mitk::Point2D p;
	//	p[0] = point.x();
	//	p[1] = point.y();
	//	mitk::Point2D p_mm;
	//	mitk::Point3D position;

	//	this->GetRenderWindow1()->GetRenderer()->GetDisplayGeometry()->ULDisplayToDisplay(p, p);
	//	this->GetRenderWindow1()->GetRenderer()->GetDisplayGeometry()->DisplayToWorld(p, p_mm);
	//	this->GetRenderWindow1()->GetRenderer()->GetDisplayGeometry()->Map(p_mm, position);

	//	mitk::Geometry3D *geometry = myMitkImage->GetGeometry();
	//	mitk::Point3D point3D;
	//	geometry->WorldToIndex(position, point3D);

	//	//round values to get correct voxel
	//	point3D[0] = round(point3D[0]);
	//	point3D[1] = round(point3D[1]);
	//	point3D[2] = round(point3D[2]);

	//	return point3D;
	//}

private:
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
};

#endif // ! MPIP_MITK_VIEWER_H