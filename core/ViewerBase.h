#ifndef VIEWER_BASE_H
#define VIEWER_BASE_H

#include <QWidget>

class ViewerBase
{
public:
	ViewerBase();

	virtual void Display(QString imagePath, QString overlayPath = QString());
	virtual void ChangeOpacity(float value);
	virtual bool RemoveImageOrOverlayIfLoaded(QString path);
	virtual void SaveOverlayToFile(QString fullPath);
};

#endif // ! VIEWER_BASE_H