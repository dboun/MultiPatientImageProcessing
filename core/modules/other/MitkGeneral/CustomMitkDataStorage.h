#ifndef CUSTOM_MITK_DATA_STORAGE_H
#define CUSTOM_MITK_DATA_STORAGE_H

#ifdef EXPORTING
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif

#include <QObject>

#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>

#include "DataManager.h"
#include "DataViewBase.h"

/** class CustomMitkDataStorage
*     Extending mitk::StandaloneDataStorage to hold the data of the currently selected subject.
*     This happens automatically and the class only listens to DataView announced changes.
*     It can also be used transparently to get/save the images from/to file
*     and update DataManager if the data doesn't belong to the current subject. 
*/
class /*MITKCORE_EXPORT*//*DECLSPEC*/ CustomMitkDataStorage : public QObject, public mitk::StandaloneDataStorage
{
	Q_OBJECT

public:
    mitkClassMacro(CustomMitkDataStorage, mitk::StandaloneDataStorage);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    /** CustomMitkDataStorage needs to be initialized with DataManager the first time */
    static CustomMitkDataStorage* CreateInstance(DataManager* dataManager);

    static CustomMitkDataStorage* GetInstance();

    void SetAppNameShort(QString appNameShort);

    /** DataViewBase should be connected with this before connecting with anything else */
    void SetDataView(DataViewBase* dataView);

    /** If the uid doesn't exist, the image will not be added */
    long AddMitkImageToSubject(long uid, mitk::Image::Pointer mitkImage, 
        QString path = QString(), QString specialRole = QString(), 
        QString name = QString(), bool external = false, bool visibleInDataView = true 
    );

    /** If the uid doesn't exist, the image will not be added */
    long AddMitkLabelSetImageToSubject(long uid, mitk::LabelSetImage::Pointer mitkImage, 
        QString path = QString(), QString specialRole = QString(), 
        QString name = QString(), bool external = false, bool visibleInDataView = true 
    );

    /** Pass uid=-1 to add it to the current subject */
    long AddEmptyMitkLabelSetImageToSubject(long uid, 
        QString specialRole = QString(), QString name = QString(),
        bool external = false, bool visibleInDataView = true 
    );

    /** The image will be converted to LabelSetImage with the special role specified.
     *  If it is already a LabelSetImage the labels it can optionally sync the colors (labels)
     *  from another LabelSetImage. The origin image will be removed only if any operations 
     *  happened and a new one will be added.
     @param iid the image to be converted
     @param newSpecialRole the new special role
     @param syncColors whether to copy colors (labels) from another LabelSetImage
     @param referenceIid image to get the colors (labels) from. If -1 it gets automatically picked.
     @return the new iid of the converted image (the previous gets removed)
     */
    long ReAddAsLabelSetImage(long iid, QString newSpecialRole = "Segmentation", 
        bool syncColors = true, long referenceIid = -1
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
    void WriteChangesToFileForAllImagesOfCurrentSubject();

public slots:
    /** Slots for DataView */
    void SelectedSubjectChangedHandler(long uid); // uid == -1 if nothing is selected
	void DataAddedForSelectedSubjectHandler(long iid);
	void DataRemovedFromSelectedSubjectHandler(long iid);
	void DataRequestedAsSeedsHandler(long iid);
	void DataRequestedAsSegmentationHandler(long iid);
	void ExportDataHandler(long iid, QString fileName);

signals:
    /** This is to inform mitk based things that a node is added.
     *  Listen to DataView::DataRemoved(long) to know when a node is deleted.
     *  That's because the node would be null */
    void MitkLoadedNewNode(long iid, mitk::DataNode::Pointer dataNode);

protected:
	virtual void AddToDataStorage(long iid);

    /** This is called before removing something from the DataStorage */
    void WriteChangesToFileIfNecessary(mitk::DataNode::Pointer dataNode);

    /** This is used internally by AddMitk[...]ImageToSubject() */
    long AddMitkNodeToSubject(long uid, mitk::DataNode::Pointer dataNode, 
        QString path, QString specialRole, QString type, QString name,
        bool external, bool visibleInDataView 
    );

    CustomMitkDataStorage();

    ~CustomMitkDataStorage();

    static DataManager* m_DataManager;
    static long         m_CurrentSubjectID;

    /** In this map nodes are added while they wait for DataManager to update.
     *  Then they are added to the mitk::DataStorage and removed from the map.
     *  Mostly used by Add[...]Mitk[...]ImageToSubject()
     */
    static std::map<QString, mitk::DataNode::Pointer> m_NodesWaitMap;

private:
    template<class T, class U>
	std::vector<T> IdsOfMap(std::map<T, U>& map)
	{
		std::vector<T> keys;
		keys.reserve(map.size());
		for (auto const& imap : map) {
			keys.push_back(imap.first);
		}
		return keys;
	}

    QString m_AppNameShort = "MPIP";
};

#endif // ! CUSTOM_MITK_DATA_STORAGE_H
