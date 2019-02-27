#include "Scheduler.h"

#ifdef BUILD_GEODESIC_TRAINING
#include "ApplicationGeodesicTrainingSegmentation.h"
#endif

Scheduler::Scheduler(QObject *parent) : SchedulerBase(parent)
{

}

void Scheduler::ThreadJob(long uid, std::vector<long> &iids, const int customFlag)
{
#ifdef BUILD_GEODESIC_TRAINING
	qDebug() << QString("Thread started for: ") << QString::number(uid);

	// Find input images and mask
	std::vector<std::string> inputImagesPaths;
	std::string maskPath = "";
	std::string subjectDirectoryPath = m_DataManager->GetSubjectPath(uid).toStdString();

	for (const long& iid : iids)
	{
		QString dataPath = m_DataManager->GetDataPath(iid);
		QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid);
	
		if (dataSpecialRole == "Mask")
		{
			maskPath = dataPath.toStdString();
		}
		else if (dataSpecialRole != "Segmentation") {
			inputImagesPaths.push_back(dataPath.toStdString());
		}
	}

	// Check requirements
	if (inputImagesPaths.size() == 0 || maskPath == "")
	{
		qDebug() << "GeodesicTraining not running because of bad input";
		ResultFinished(uid);
		return;
	}

	emit jobQueued(uid);

	ApplicationGeodesicTrainingSegmentation<float, 3> geodesic; // TODO: Support 2D
	geodesic.SetUid(uid);
	connect(&geodesic, SIGNAL(ProgressUpdateUI(long, QString, int)), 
		this, SLOT(progressUpdateFromApplication(long, QString, int))
	);

	geodesic.SetInputImages(inputImagesPaths);
	geodesic.SetLabels(maskPath);
	geodesic.SetOutputPath(subjectDirectoryPath + std::string("/Segmentation"));
	geodesic.SetSaveAll(true);
	geodesic.SetTimerEnabled(true);
	geodesic.SetVerbose(true);
	geodesic.SetNumberOfThreads(8);
	geodesic.Execute();

	QString outputSegmentationPath = QString::fromStdString(subjectDirectoryPath) + QString("/Segmentation/labels_res.nii.gz");
	m_DataManager->AddDataToSubject(uid, outputSegmentationPath, "Segmentation");

#endif
	ResultFinished(uid);
}