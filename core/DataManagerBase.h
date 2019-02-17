#define DATA_MANAGER_BASE_H
#define DATA_MANAGER_BASE_H

#include <QString>
#include <QObject>

#include <vector>
#include <map>
#include <mutex>

/** class DataManagerBase
*     This is a base class used to provide basic data
*     storage functionality. 
*     It can be inherited to provide a viewer for the subjects/images, 
*     but it can be also be used by itself.
*/
class DataManagerBase : public QObject
{
	Q_OBJECT

public:
	DataManagerBase() {}

	/** struct ImageData
	*   contains information about an image
	*   iid is the unique id of the image 
	*/
	typedef struct {
		long iid;
		QString imageName;
		QString imagePath;
		QString specialRole;
	} ImageData;

	/** struct SubjectData
	*   contains information about a subject (and it's images)
	*   uid is the unique id of the subject 
	*/
	typedef struct {
		long uid;
		QString subjectName;
		QString subjectPath;
		std::map< long, ImageData > subjectImages;
	} SubjectData;

	/** Special role will be what is displayed instead of the 
	*   image name. i.e. Mask
	*/
	virtual void AddSpecialRole(QString specialRole)
	{
		m_SpecialRoles << specialRole;
	}

	// Subject Getters

	std::vector<long> GetAllSubjectIds()
	{
		// TODO
	}

	std::vector<long> GetAllImageIdsOfSubject(long uid)
	{
		QStringList paths;
		// TODO
	}

	QString GetSubjectPath(long uid)
	{
		// TODO
	}

	QString GetSubjectName(long uid)
	{
		// TODO
	}

	// Image Getters

	long GetSubjectIdFromImageId(long uid)
	{

	}

	QString GetImagePath(long iid)
	{
		// TODO
	}

	QString GetImageName(long iid)
	{
		// TODO
	}

	QString GetImageSpecialRole(long iid)
	{
		// TODO
	}

	bool imageHasSpecialRole(long iid)
	{
		// TODO
	}

public slots:
	
	/** Add a new subject by providing the path to the directory containing the subject's images
	@param dirPath the directory
	@param subjectName deduced from the path if not provided 
	@return the uid of the subject. -1 if adding failed
	*/
	virtual long AddSubjectByDirectoryPath(QString dirPath, QString subjectName = QString())
	{
		// TODO
	}

	/** Add new subject
	@param subjectPath the path to the directory containing the images 
	@param subjectName deduced from the path if not provided 
	@return the uid of the subject. -1 if adding failed
	*/
	long AddSubject(QString subjectPath, QStringList imagesPaths, 
		QString subjectName = QString())
	{
		// TODO
	}

	/** Remove a subject
	@param uid the unique id of the subject
	*/
	void RemoveSubject(long uid)
	{
		// TODO
	}

	/** Add image to subject
	@param uid the subject's id. Use AddSubject for new subject
	@param imagePath the path to the image
	@param specialRole if the image has a special role like mask
	@param imageName deduced from the path if not provided
	@return the image's id. -1 if adding failed
	*/
	long AddImageToSubject(long uid, QString imagePath, 
		QString specialRole = QString(), QString imageName = QString())
	{
		// TODO
	}

	/** Remove image
	@param iid the image's id.
	*/
	void RemoveImage(long iid)
	{
		// TODO
	}

signals:
	SubjectChanged(long uid);
	SubjectContentChanged(long uid);
	SubjectRemoved(long uid);
	SubjectAdded(long uid);

protected:
	std::map<long, SubjectData> m_Subjects; 
	std::map<long, long>        m_SubjectIdByImageId;
	QStringList                 m_SpecialRoles;
	
private:
	long uidNextToGive = 0;
	long iidNextToGive = 0;
	std::mutex m_Mutex;
};

#endif // ! DATA_MANAGER_BASE_H