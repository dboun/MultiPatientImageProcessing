#ifndef VIEWER_BASE_H
#define VIEWER_BASE_H

#include <QWidget>

class ViewerBase : public QWidget
{
	Q_OBJECT

public:
	ViewerBase();

	virtual void Display(QString imagePath, QString overlayPath = QString());
	virtual void ChangeOpacity(float value);
	virtual bool RemoveImageOrOverlayIfLoaded(QString path);
	virtual void SaveOverlayToFile(QString fullPath);

signals:
	void OverlayChangedFor(QString imagePath); // Whenever the user creates or edits a mask
											   // Should change button to 'Save Mask and Run'
};

#endif // ! VIEWER_BASE_H