#include "DataManager.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

DataManager::DataManager(QObject *parent) : QObject(parent)
{
	// For internal cache subject path
	QString appDataDir = QStandardPaths::standardLocations(
		QStandardPaths::AppDataLocation
	).at(0);
	QDir dir(appDataDir);
	if (dir.exists())
	{
		// Delete old one if it exists
		dir.removeRecursively();
	}
	qDebug() << "(DataManager) AppData dir: " << appDataDir;
	if (!QDir(appDataDir).exists())
	{
		QDir().mkpath(appDataDir);
	}
}

DataManager::~DataManager()
{
	// Remove data in reverse order to avoid reloading everything to the viewer
	auto uids = this->GetAllSubjectIds();
	long uid;

	for (long i = uids.size()-1; i >= 0; i--)
	{
		uid = uids[i];
		this->RemoveSubject(uid);
	}

	// For internal cache subject path
	QString appDataDir = QStandardPaths::standardLocations(
		QStandardPaths::AppDataLocation
	).at(0);
	QDir dir(appDataDir);
	if (dir.exists())
	{
		qDebug() << "Deleting" << m_AppNameShort << "AppData directory";
		dir.removeRecursively();
	}
}

void DataManager::SetAcceptedFileTypes(QStringList& acceptedFileTypes)
{
	m_AcceptedFileTypes = acceptedFileTypes;
}

void DataManager::SetAppNameShort(QString appNameShort)
{
	m_AppNameShort = appNameShort;
}

QStringList DataManager::GetAcceptedFileTypes()
{
	return m_AcceptedFileTypes;
}

std::mutex* DataManager::GetSubjectEditMutexPointer(long uid)
{
	if (m_SubjectEditMutex.find(uid) != m_SubjectEditMutex.end())
	{
		return m_SubjectEditMutex[uid].get();
	}
	return nullptr;
}

std::vector<long> DataManager::GetAllSubjectIds()
{
	return IdsOfMap<long,Subject>(m_Subjects); 
}

QString DataManager::GetSubjectName(long uid)
{
	if (m_Subjects.find(uid) != m_Subjects.end())
	{
		return m_Subjects[uid].name;
	}
	return QString();
}

QString DataManager::GetSubjectPath(long uid)
{
	if (m_Subjects.find(uid) != m_Subjects.end())
	{
		return m_Subjects[uid].path;	
	}
	return QString();
}

QString DataManager::GetOriginalSubjectPath(long uid)
{
	if (m_Subjects.find(uid) != m_Subjects.end())
	{
		return m_Subjects[uid].originalPath;
	}
	return QString();
}

std::vector<long> DataManager::GetAllDataIdsOfSubject(long uid)
{
	if (m_Subjects.find(uid) != m_Subjects.end())
	{
		return m_Subjects[uid].dataIds;			
	}
	return std::vector<long>();
}

std::vector<long> DataManager::GetAllDataIdsOfSubjectWithSpecialRole(long uid, QString specialRole)
{
	if (m_Subjects.find(uid) != m_Subjects.end())
	{
		std::vector<long> ret;
		for (const long& iid : m_Subjects[uid].dataIds)
		{
			if (m_Data[iid].specialRole == specialRole)
			{
				ret.push_back(iid);
			}
		}
		return ret;
	}
	return std::vector<long>();
}

QString DataManager::GetDataName(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].name;
	}
	return QString();
}

QString DataManager::GetDataPath(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].path;
	}
	return QString();
}

QString DataManager::GetDataType(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].type;
	}
	return QString();
}

QString DataManager::GetDataSpecialRole(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].specialRole;
	}
	return QString();
}

bool DataManager::GetDataIsExternal(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].external;
	}
	return false;
}

bool DataManager::GetDataIsVisibleInDataView(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].visibleInDataView;
	}
	return false;
}

long DataManager::GetSubjectIdFromDataId(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid].subjectId;
	}
	return -1;
}

long DataManager::AddSubjectAndDataByDirectoryPath(QString dirPath, QString subjectName)
{
	QStringList allFiles;
	FindAllFilesRecursively(dirPath, allFiles);

	if (allFiles.isEmpty())
	{
		return -1;
	}

	long uid = AddSubject(dirPath, subjectName);

	for (const auto& filePath : allFiles)
	{
		// // Find if the data has a special role (at least a one that we define)
		// // If there is a subdirectory called APPNAMESHORT_X
		// // Then X is the special role of the images contained in this subdirectory
		// QString filePathTemp = filePath;
		// filePathTemp.replace("\\", "/", Qt::CaseSensitive);
		// QStringList filePathSplit = filePathTemp.split("/");
		// QString parentDirOfFile = filePathSplit.value(filePathSplit.length() - 2);
		
		// qDebug() << "Parent dir of file" << parentDirOfFile;
		
		// QString specialRole;

		// if (parentDirOfFile.startsWith(m_AppNameShort, Qt::CaseSensitive))
		// {
		// 	specialRole = parentDirOfFile.right(
		// 		parentDirOfFile.length() - m_AppNameShort.length() - 1
		// 	);
		// 	qDebug() << "Special Role detected" << specialRole;
		// }

		// Actually add the data if necessary
		AddDataToSubject(uid, filePath, /*specialRole*/"", "Image");
	}

	return uid;
}

long DataManager::AddSubject(QString subjectPath, QString subjectName)
{
	std::unique_lock<std::mutex> ul(m_Mutex);

	long uid = uidNextToGive++;
	m_Subjects[uid] = Subject();

	if (subjectName != QString())
	{
		m_Subjects[uid].name = subjectName;
	}
	else {
		QString name = QString::fromStdString(
			subjectPath.toStdString().substr(
				subjectPath.toStdString().find_last_of("/\\") + 1
				)
			);
		m_Subjects[uid].name = name;
	}

	m_Subjects[uid].originalPath = subjectPath;

	// For internal cache subject path
	m_Subjects[uid].path = QStandardPaths::standardLocations(
		QStandardPaths::AppDataLocation
	).at(0) + QString("/") + QString::number(uid);
	if (!QDir(m_Subjects[uid].path).exists())
	{
		QDir().mkpath(m_Subjects[uid].path);
	}
	qDebug() << "DataManager::AddSubject intenal path" << m_Subjects[uid].path;

	m_SubjectEditMutex[uid] = std::unique_ptr<std::mutex>(new std::mutex());
	
	ul.unlock();
	qDebug() << "Emit DataManager::SubjectAdded()";
	emit SubjectAdded(uid);

	return uid;
}

void DataManager::RemoveSubject(long uid)
{
	std::unique_lock<std::mutex> ul(m_Mutex);

	if (m_Subjects.find(uid) == m_Subjects.end())
	{
		return;
	}

	for (long& iid :  m_Subjects[uid].dataIds)
	{
		m_Data.erase(iid);
	}

	m_Subjects.erase(uid);
	m_SubjectEditMutex.erase(uid);

	ul.unlock();
	emit SubjectRemoved(uid);
}

long DataManager::AddDataToSubject(long uid, QString path, QString specialRole, 
	QString type, QString name, bool external, bool visibleInDataView)
{
	std::unique_lock<std::mutex> ul(m_Mutex);
	
	m_Data[iidNextToGive] = Data();
	m_Data[iidNextToGive].path = path;
	m_Data[iidNextToGive].specialRole = specialRole;
	m_Data[iidNextToGive].type = type;
	m_Data[iidNextToGive].external = external;
	m_Data[iidNextToGive].visibleInDataView = visibleInDataView;

	if (name != QString())
	{
		m_Data[iidNextToGive].name = name;
	}
	else {
		QString nameAuto = QString::fromStdString(
			path.toStdString().substr(
				path.toStdString().find_last_of("/\\") + 1
				)
			);

		m_Data[iidNextToGive].name = nameAuto;
	}

	m_Data[iidNextToGive].subjectId = uid;
	m_Subjects[uid].dataIds.push_back(iidNextToGive);

	long iid = iidNextToGive++;

	ul.unlock();
	qDebug() << "Emit DataManager::SubjectDataChanged()";
	emit SubjectDataChanged(uid);

	return iid;
}

void DataManager::RemoveData(long iid)
{
	std::unique_lock<std::mutex> ul(m_Mutex);

	if (m_Data.find(iid) == m_Data.end())
	{
		return;
	}

	// Erase the iid from the subject's dataIds
	long uid = m_Data[iid].subjectId;
	m_Subjects[uid].dataIds.erase(
		std::remove(
			m_Subjects[uid].dataIds.begin(), 
			m_Subjects[uid].dataIds.end(), 
			iid
			), 
		m_Subjects[uid].dataIds.end()
	);

	m_Data.erase(iid);

	if (m_Subjects[uid].dataIds.size() == 0)
	{
		m_Subjects.erase(uid);
		ul.unlock();
		emit SubjectRemoved(uid);	
	}
	else {
		ul.unlock();
		emit SubjectDataChanged(uid);
	}
}

void DataManager::FindAllFilesRecursively(QString directoryPath, QStringList& allFiles)
{
	qDebug() << QString("Trying dir: ") << directoryPath;
	QDir dir = QDir(directoryPath);

	// Find all files in this directory
	QStringList files = dir.entryList(m_AcceptedFileTypes,
		QDir::Files | QDir::NoSymLinks);

	// Find all subdirectories
	dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	QStringList subdirectories = dir.entryList();

	// Add all files to the patient list
	for (const auto& file : files)
	{
		qDebug() << QString("Found file") << file;
		allFiles << directoryPath + QString("/") + file;
	}

	// Do the same for subdirectories
	for (const auto& subdir : subdirectories)
	{
		qDebug() << "Found subdir" << subdir;
		FindAllFilesRecursively(directoryPath + QString("/") + subdir, allFiles);
	}
}