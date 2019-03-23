#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <QString>
#include <QObject>
#include <QString>
#include <QDir>

#include <vector>
#include <map>
#include <mutex>
#include <memory>

/** class DataManager
*     This is a class used to provide basic data
*     storage functionality. 
*     It can be used by a viewer for the subjects/images, 
*     but it can be also be used by itself.
*/
class DataManager : public QObject
{
	Q_OBJECT

public:
	DataManager(QObject *parent = nullptr);
	~DataManager();

	/** struct Data
	*   contains information about data for a subject
	*   iid is the unique id of the data 
	*   Special role will be what is displayed instead of the 
	*   image name. i.e. Mask
	*/
	typedef struct {
		long iid;
		QString name;
		QString path;
		QString type;
		QString specialRole;
		long    subjectId;
	} Data;

	/** struct Subject
	*   contains information about a subject (and it's images etc)
	*   uid is the unique id of the subject 
	*/
	typedef struct {
		long uid;
		QString name;
		QString path;
		QString originalPath;
		std::vector<long> dataIds;
	} Subject;

	void SetAcceptedFileTypes(QStringList& acceptedFileTypes);
	void SetAppNameShort(QString appNameShort);
	QStringList GetAcceptedFileTypes();

	std::mutex* GetSubjectEditMutexPointer(long uid);

	// Subject Getters

	std::vector<long> GetAllSubjectIds();
	QString GetSubjectName(long uid);
	QString GetSubjectPath(long uid);
	QString GetOriginalSubjectPath(long uid);
	std::vector<long> GetAllDataIdsOfSubject(long uid);
	std::vector<long> GetAllDataIdsOfSubjectWithSpecialRole(long uid, QString specialRole);

	// Data Getters

	QString GetDataName(long iid);
	QString GetDataPath(long iid);
	QString GetDataType(long iid);
	QString GetDataSpecialRole(long iid);
	long GetSubjectIdFromDataId(long iid);

public slots:
	
	/** Add a new subject by providing the path to the directory containing the subject's images
	@param dirPath the directory
	@param subjectName deduced from the path if not provided 
	@return the uid of the subject. -1 if adding failed
	*/
	virtual long AddSubjectAndDataByDirectoryPath(QString dirPath, QString subjectName = QString());

	/** Add new subject
	@param subjectPath the path to the directory containing the images 
	@param subjectName deduced from the path if not provided 
	@return the uid of the subject. -1 if adding failed
	*/
	long AddSubject(QString subjectPath, QString subjectName = QString());

	/** Remove a subject(and its data)
	@param uid the unique id of the subject
	*/
	void RemoveSubject(long uid);

	/** Add data to subject
	@param uid the subject's id. Use AddSubject for new subject
	@param path the path to the data
	@param specialRole if the image has a special role like 'mask' or 'model'
	@param type user defined (like image or xml)
	@param name deduced from the path if not provided
	@return the image's id. -1 if adding failed
	*/
	long AddDataToSubject(long uid, QString path, QString specialRole = QString(), 
		QString type = QString(), QString name = QString());

	/** Remove image
	@param iid the data's id.
	@param silent will not emit a DataAboutToGetRemoved(long) to avoid infinite loops
	*/
	void RemoveData(long iid, bool silent = false);

signals:
	void SubjectAdded(long uid);
	void SubjectRemoved(long uid);
	void SubjectDataChanged(long uid);

	void DataAboutToGetRemoved(long iid);

protected:
	std::map<long, Subject> m_Subjects; 
	std::map<long, Data>    m_Data;
	QStringList             m_SpecialRoles;
	
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

	void FindAllFilesRecursively(QString directoryPath, QStringList& allFiles);

	long uidNextToGive = 0;
	long iidNextToGive = 0;
	std::mutex  m_Mutex;
	std::map< long, std::unique_ptr<std::mutex> >  m_SubjectEditMutex;
	QStringList m_AcceptedFileTypes = QStringList() << "*";
	QString     m_AppNameShort      = "MPIP";
};

#endif // ! DATA_MANAGER_H