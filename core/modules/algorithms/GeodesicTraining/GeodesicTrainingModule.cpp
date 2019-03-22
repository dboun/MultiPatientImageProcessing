#include "GeodesicTrainingModule.h"

#include <QDebug>

#include <vector>
#include <string>
#include <mutex>

#include "GeodesicTrainingQt.h"

GeodesicTrainingModule::GeodesicTrainingModule(QObject *parent) : AlgorithmModuleBase(parent)
{
    this->SetSeverity(AlgorithmModuleBase::SEVERITY::HIGH);
    this->SetAlgorithmName("Geodesic Training Segmentation");
    this->SetAlgorithmNameShort("GeodesicTraining");
}

GeodesicTrainingModule::~GeodesicTrainingModule()
{

}

void GeodesicTrainingModule::GeodesicTrainingProgressUpdateHandler(QString message, int progress)
{
    emit ProgressUpdateUI(m_Uid, message, progress);
}

void GeodesicTrainingModule::Algorithm()
{
    if (m_Uid == -1)
    {
        // Other checks also happen in AlgorithmModuleBase::Run()
        emit ProgressUpdateUI(m_Uid, "Error", -1);
        emit AlgorithmFinishedWithError(this, "No subject selected");
        return;
    }

    DataManager* dm = this->GetDataManager();

    int numberOfImages = 0, numberOfMasks = 0;
    
    std::vector<std::string> images;
    std::string mask;

    auto iids = dm->GetAllDataIdsOfSubject(m_Uid);

    for (const long& iid : iids)
	{
		QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid); 
        QString dataPath = m_DataManager->GetDataPath(iid);

		if (dataSpecialRole == "Mask" && dataPath.endsWith(".nii.gz", Qt::CaseSensitive)) {
			numberOfMasks++;
            qDebug() << "Found nifti mask, with path" << dataPath;
            mask = dataPath.toStdString();
		}
		else if (dataSpecialRole != "Segmentation" && dataPath.endsWith(".nii.gz", Qt::CaseSensitive))
		{
			numberOfImages++;
            images.push_back(dm->GetDataPath(iid).toStdString());
		}
	}

	if (numberOfMasks == 0)
	{
        emit ProgressUpdateUI(m_Uid, "Error", -1);
		emit AlgorithmFinishedWithError(this, "No mask drawn");
		return;
	}
	if (numberOfMasks > 1)
	{
        emit ProgressUpdateUI(m_Uid, "Error", -1);
        emit AlgorithmFinishedWithError(this, "Multiple masks. " +
            QString("Please remove all but one masks ") +
            QString("Right click & Remove won't delete an image from the disk).")
        );
		return;
	}
	if (numberOfImages == 0)
	{
        emit ProgressUpdateUI(m_Uid, "Error", -1);
        emit AlgorithmFinishedWithError(this, "No images. Please load some images.");
		return;
	}

	// TODO: 2D
    GeodesicTrainingQt<3>* geodesicTraining = new GeodesicTrainingQt<3>(this);

    connect(geodesicTraining, SIGNAL(GeodesicTrainingProgressUpdate(QString, int)),
        this, SLOT(GeodesicTrainingProgressUpdateHandler(QString, int))
    );

    QString outputPath = dm->GetSubjectPath(m_Uid) + 
        "/" + this->GetAppNameShort() +
        "/" + this->GetAppNameShort() + "_Segmentation";

    qDebug() << "GeodesicTraining will use " << m_IdealNumberOfThreads << " threads";
    qDebug() << "GeodesicTraining will use mask " << mask.c_str();


    std::unique_lock<std::mutex> ul(*this->GetDataManager()->GetSubjectEditMutexPointer(m_Uid));
    std::vector<typename itk::Image<float, 3>::Pointer> inputImagesITK;
    for (const std::string& image : images)
    {
        inputImagesITK.push_back(cbica::ReadImage<itk::Image<float, 3>>(image));
    }

    geodesicTraining->SetInputImages(inputImagesITK);
    geodesicTraining->SetLabels(cbica::ReadImage<itk::Image<int, 3>>(mask));
    geodesicTraining->SetOutputPath(outputPath.toStdString());
    geodesicTraining->SetNumberOfThreads(m_IdealNumberOfThreads);
    geodesicTraining->SaveOnlyNormalSegmentation(true, "segmentation");
    geodesicTraining->SetVerbose(true);
    //geodesicTraining->SetTimerEnabled(true);

    ul.unlock();
    auto result = geodesicTraining->Execute();

    if (result->ok)
    {
        this->GetDataManager()->AddDataToSubject(this->GetUid(), 
            outputPath + QString("/segmentation.nii.gz"), "Segmentation"
        );
    }
    else {
        qDebug() << "GeodesicTraining finished with internal error";
        emit ProgressUpdateUI(m_Uid, "Error", -1);
        emit AlgorithmFinishedWithError(this, result->errorMessage.c_str());
    }

    delete geodesicTraining;
}