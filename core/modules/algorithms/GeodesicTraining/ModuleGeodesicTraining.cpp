#include "ModuleGeodesicTraining.h"

#include <vector>
#include <string>

#include "GeodesicTrainingQt.h"

GeodesicTrainingModule::GeodesicTrainingModule(QObject *parent) : AlgorithmModuleBase(parent)
{
    this->SetSeverity(AlgorithmModuleBase::SEVERITY::HIGH);
    this->SetAlgorithmName("Geodesic Training Segmentation");
    this->SetAlgorithmNameShort("GeodesicTraining");
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

		if (dataSpecialRole == "Mask") {
			numberOfMasks++;
            mask = dm->GetDataPath(iid).toStdString();
		}
		else if (dataSpecialRole != "Segmentation")
		{
			numberOfImages++;
            images.push_back(dm->GetDataPath(iid).toStdString());
		}
	}

	if (numberOfMasks == 0)
	{
		emit AlgorithmFinishedWithError(this, "No mask drawn");
		return;
	}
	if (numberOfMasks > 1)
	{
        emit AlgorithmFinishedWithError(this, "Multiple masks. " +
            QString("Please remove all but one masks ") +
            QString("Right click & Remove won't delete an image from the disk).")
        );
		return;
	}
	if (numberOfImages == 0)
	{
        emit AlgorithmFinishedWithError(this, "No images. Please load some images.");
		return;
	}

	// TODO: 2D
    GeodesicTrainingQt<3>* geodesicTraining = new GeodesicTrainingQt<3>(this);

    connect(geodesicTraining, SIGNAL(GeodesicTrainingProgressUpdate(QString, int)),
        this, SLOT(GeodesicTrainingProgressUpdate(QString, int))
    );

    QString outputPath = dm->GetSubjectPath(m_Uid) + 
        "/" + this->GetAppNameShort() + "_Segmentation";

    geodesicTraining->SetInputImages(images);
    geodesicTraining->SetLabels(mask);
    geodesicTraining->SetOutputPath(outputPath.toStdString());
    geodesicTraining->SetSaveAll(true);
    geodesicTraining->Execute();

    delete geodesicTraining;
}
