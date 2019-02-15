#include "MainWindow.h"
#include <QSurfaceFormat>

#include "QVTKOpenGLWidget.h"
#include <QApplication>
#include "QmitkRegisterClasses.h"

int main(int argc, char *argv[])
{
  QmitkRegisterClasses();

	QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

	QApplication a(argc, argv);
	MainWindow w;
	
	// For darker color
	QPalette pal = QPalette();
	pal.setColor(QPalette::Background, Qt::darkGray);
	w.setAutoFillBackground(true);
	w.setPalette(pal);
    
	w.show();

	return a.exec();
}
