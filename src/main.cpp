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
	a.setAttribute(Qt::AA_EnableHighDpiScaling, true);
    a.setWindowIcon(QIcon(":/mll_icon.ico"));
	MainWindow w;
	w.show();

	// For global application theme
	QFile File(":dark.qss");
    if (File.open(QFile::ReadOnly))
    {
        QString StyleSheet = QLatin1String(File.readAll());
        a.setStyleSheet(StyleSheet);
	}

	return a.exec();
}
