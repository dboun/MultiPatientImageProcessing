#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
	
	QPalette pal = QPalette();
	pal.setColor(QPalette::Background, Qt::darkGray);
	w.setAutoFillBackground(true);
	w.setPalette(pal);
    
	w.show();

    return a.exec();
}
