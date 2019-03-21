#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QColor>

#ifdef BUILD_MODULE_MitkImageViewer
#include "QmitkRegisterClasses.h"
//#include "QVTKOpenGLWidget.h"
#endif

int main(int argc, char *argv[])
{
#ifdef BUILD_MODULE_MitkImageViewer
	QmitkRegisterClasses();
	//QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());
#endif

	QApplication a(argc, argv);
	MainWindow w;
	
	// // For darker color
	// QPalette pal = QPalette();
	// pal.setColor(QPalette::Background, QColor(33, 33, 33));
	// w.setAutoFillBackground(true);
	// w.setPalette(pal);
    
	w.show();

	QFile File(":dark.qss");
    if (File.open(QFile::ReadOnly))
    {
        QString StyleSheet = QLatin1String(File.readAll());
        a.setStyleSheet(StyleSheet);
	}

	return a.exec();
}