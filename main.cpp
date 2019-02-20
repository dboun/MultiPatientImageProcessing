#include "mainwindow.h"
//#include <QSurfaceFormat>

#include <QApplication>
#include <QColor>

#ifdef BUILD_VIEWER
#include "QmitkRegisterClasses.h"
//#include "QVTKOpenGLWidget.h"
#endif

int main(int argc, char *argv[])
{
#ifdef BUILD_VIEWER
	QmitkRegisterClasses();
	//QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());
#endif

	QApplication a(argc, argv);
	MainWindow w;
	
	// For darker color
	QPalette pal = QPalette();
	pal.setColor(QPalette::Background, QColor(33, 33, 33));
	w.setAutoFillBackground(true);
	w.setPalette(pal);
    
	w.show();

	return a.exec();
}
