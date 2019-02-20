#ifndef DATA_VIEW_BASE_H
#define DATA_VIEW_BASE_H

#include <QWidget>

#include "DataManager.h"

class DataViewBase : public QWidget
{
	Q_OBJECT

public:
	DataViewBase(QWidget* parent = nullptr);

	void SetDataManager(DataManager* dataManager);

public slots:
	// Override these methods to provide functionality to the viewer
	virtual void SubjectAddedHandler(long uid);
	virtual void SubjectRemovedHandler(long uid);
	virtual void SubjectDataChangedHandler(long uid);
	virtual void UpdateProgressHandler(long uid, int progress);

signals:
	void SelectedSubjectChanged(long uid); // uid == -1 if nothing is selected
	void SelectedDataChanged(long iid);
	void DataAddedForSelectedSubject(long iid);
	void DataRemovedFromSelectedSubject(long iid);
	void DataCheckedStateChanged(long iid, bool checkState); // Optional
	
protected:
	DataManager* m_DataManager;
	QStringList m_AcceptedFileTypes = QStringList() << "*";
};

#endif // ! DATA_VIEW_BASE_H