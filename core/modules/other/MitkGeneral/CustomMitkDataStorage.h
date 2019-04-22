#ifndef CUSTOM_MITK_DATA_STORAGE_H
#define CUSTOM_MITK_DATA_STORAGE_H

#include <QObject>

#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>

#include "DataManager.h"
#include "DataViewBase.h"

class CustomMitkDataStorage : public QObject, public mitk::StandaloneDataStorage
{
	Q_OBJECT

public:
    static CustomMitkDataStorage& CreateInstance(DataManager* dataManager);

    static CustomMitkDataStorage& GetInstance();

	~CustomMitkDataStorage();

    /** DataViewBase should be connected with this before connecting with anything else */
    void SetDataView(DataViewBase* dataView);

    long AddMitkImageToSubject(long uid, mitk::Image::Pointer mitkImage, 
        QString specialRole = QString(), QString type = QString(), QString name = QString(),
        bool external = false, bool visibleInDataView = true 
    );

    long AddEmptyMitkImageToSubject(long uid, 
        QString specialRole = QString(), QString type = QString(), QString name = QString(),
        bool external = false, bool visibleInDataView = true 
    );

    /** GetImage returns from DataStorage if the image is from the current subject
        otherwise it normally loads it (outside the DataStorage) and returns it */
    mitk::Image::Pointer GetImage(long iid); 

    /** GetLabelSetImage returns from DataStorage if the image is from the current subject
        otherwise it normally loads it (outside the DataStorage) and returns it */
    mitk::LabelSetImage::Pointer GetLabelSetImage(long iid);

    /** This should be used to force the updated images to file, 
     *  if an algorithm wants to read the from file. 
     * */
    void WriteChangesToFileIfNecessaryForAllImagesOfCurrentSubject();

public slots:
    /** Slots for DataManager */
	//void DataAboutToGetRemovedHandler(long iid);

    /** Slots for DataView */
    void SelectedSubjectChangedHandler(long uid); // uid == -1 if nothing is selected
	void DataAddedForSelectedSubjectHandler(long iid);
	void DataRemovedFromSelectedSubjectHandler(long iid);
	void DataRequestedAsMaskHandler(long iid);
	void DataRequestedAsSegmentationHandler(long iid);
	void ExportDataHandler(long iid, QString fileName);

signals:
    void MitkLoadedNewNode(long iid, mitk::DataNode::Pointer dataNode);

protected:
	virtual void AddToDataStorage(long iid);

    /** This is called before removing something from the DataStorage */
    void WriteChangesToFileIfNecessary(mitk::DataNode::Pointer dataNode);

    CustomMitkDataStorage() : QObject(nullptr) {}

    static DataManager* m_DataManager;
    static long         m_CurrentSubjectID;

    /** In this map nodes are added while they wait for DataManager to update.
     *  Then they are added to the mitk::DataStorage and removed from the map.
     *  Mostly used by AddMitk[...]ImageToSubject()
     */
    static std::map<long, mitk::DataNode::Pointer> m_NodesWaitMap;
};

#endif // ! CUSTOM_MITK_DATA_STORAGE_H
