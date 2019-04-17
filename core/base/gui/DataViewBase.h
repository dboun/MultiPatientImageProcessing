#ifndef DATA_VIEW_BASE_H
#define DATA_VIEW_BASE_H

#include <QWidget>
#include <QString>

#include "DataManager.h"
#include "GuiModuleBase.h"

class DataViewBase : public GuiModuleBase
{
	Q_OBJECT

public:
	DataViewBase(QWidget* parent = nullptr);

	long GetCurrentSubjectID();
	long GetCurrentDataID();

	void SetDataManager(DataManager* dataManager) override;
	
public slots:
	// Override these methods to provide functionality to the viewer
	virtual void SubjectAddedHandler(long uid);
	virtual void SubjectRemovedHandler(long uid);
	virtual void SubjectDataChangedHandler(long uid);
	virtual void UpdateProgressHandler(long uid, QString message, int progress);

signals:
	void SelectedSubjectChanged(long uid); // uid == -1 if nothing is selected
	void SelectedDataChanged(long iid);
	void DataAddedForSelectedSubject(long iid);
	void DataRemovedFromSelectedSubject(long iid);
	void DataCheckedStateChanged(long iid, bool checkState); // Optional

	void DataRequestedAsMask(long iid);
	void DataRequestedAsSegmentation(long iid);
	void ExportData(long iid, QString fileName);

protected:

	void SetCurrentSubjectID(long uid);
	void SetCurrentDataID(long iid);

	QStringList  m_AcceptedFileTypes = QStringList() << "*"; // Controlled by DataManager
	
	long m_CurrentSubjectID = -1;
	long m_CurrentDataID    = -1;
};

#endif // ! DATA_VIEW_BASE_H