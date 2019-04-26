#include "GeodesicTrainingWarningImageSize.h"

#include <QDebug>

#include "CustomMitkDataStorage.h"

GeodesicTrainingWarningImageSize::GeodesicTrainingWarningImageSize(QObject* parent) : 
    WarningFunctionBase(parent),
    m_DataStorage(CustomMitkDataStorage::GetInstance())
{
    this->SetName("Image size");
}

GeodesicTrainingWarningImageSize::~GeodesicTrainingWarningImageSize()
{

}

void GeodesicTrainingWarningImageSize::SetDataManager(DataManager* dataManager)
{
    WarningFunctionBase::SetDataManager(dataManager);

    if (m_DataView != nullptr)
    {
        WarningFunctionBase::SetDataView(m_DataView);
        
        // In case this was created after data were loaded
        this->SelectedSubjectChangedHandler(m_DataView->GetCurrentSubjectID());
    }
}

void GeodesicTrainingWarningImageSize::SetDataView(DataViewBase* dataView)
{   
    m_DataView = dataView;

    if (m_DataManager != nullptr)
    {
        WarningFunctionBase::SetDataView(dataView);
        
        // In case this was created after data were loaded
        this->SelectedSubjectChangedHandler(m_DataView->GetCurrentSubjectID());
    }
}

void GeodesicTrainingWarningImageSize::SelectedSubjectChangedHandler(long uid)
{
    qDebug() << "GeodesicTrainingWarningImageSize::SelectedSubjectChangedHandler" << uid;

    if (uid == -1)
    {
        this->SetOperationAllowed(false, "No subjects loaded");
        return;
    }

    auto iids = m_DataManager->GetAllDataIdsOfSubject(uid);
    if (iids.size() == 0) 
    {
        this->SetOperationAllowed(false, "No images");
        return;
    }

    long iid = -1;
    for (const long& tIid : iids)
    {
        if (m_DataManager->GetDataType(tIid) == "Image")
        {
            iid = tIid;
            break;
        }
    }

    if (iid == -1)
    {
        this->SetOperationAllowed(false, "No actual images");
        return;
    }

    InspectImage(iid);
}

void GeodesicTrainingWarningImageSize::DataAddedForSelectedSubjectHandler(long iid)
{
    if (m_DataManager->GetDataType(iid) == "Image")
    {
        InspectImage(iid);
    }
}

void GeodesicTrainingWarningImageSize::DataRemovedFromSelectedSubjectHandler(long iid)
{
    this->SelectedSubjectChangedHandler(m_DataView->GetCurrentSubjectID());
}

void GeodesicTrainingWarningImageSize::InspectImage(long iid)
{
    auto image = m_DataStorage->GetImage(iid);
    auto descriptor = image->GetImageDescriptor();
    unsigned int dimensions = descriptor->GetNumberOfDimensions();

    if (dimensions < 2 || dimensions > 3)
    {
        this->SetOperationAllowed(false, 
            "Only 2D and 3D images are supported"
        );
        return;
    }

    auto dimensionsArray = descriptor->GetDimensions();

    long uid  = m_DataManager->GetSubjectIdFromDataId(iid);
    auto iids = m_DataManager->GetAllDataIdsOfSubject(uid);

    for (const long& tIid : iids)
    {
        if (tIid == iid || m_DataManager->GetDataType(tIid) != "Image") { continue; }

        auto tImage = m_DataStorage->GetImage(tIid);
        auto tDescriptor = tImage->GetImageDescriptor();

        if (dimensions != tDescriptor->GetNumberOfDimensions())
        {
            this->SetOperationAllowed(false, 
                "Not all images have the same number of dimensions"
            );
            return;
        }

        auto tDimensionsArray = tDescriptor->GetDimensions();

        for (unsigned int i = 0; i < dimensions; i++)
        {
            if (dimensionsArray[i] != tDimensionsArray[i])
            {
                this->SetOperationAllowed(false, 
                    "Not all images have the same size"
                );
                return;      
            }
        }
    }

    // All important checks have passed
    this->SetOperationAllowed(true);

    // Check for warnings
    QStringList warnings;

    long numberOfPixels = 1;
    for (unsigned int i = 0; i < dimensions; i++)
    {
        numberOfPixels *= dimensionsArray[i];
    }
    qDebug() << "Number of pixels: " << numberOfPixels;

    if (numberOfPixels > 10000000)
    {
        warnings.push_back(
            QString("Images are too large and the algorithm will probably take too long. ") + 
            QString("Consider subsampling them first.")
        );
    }

    this->UpdateWarnings(warnings);
}