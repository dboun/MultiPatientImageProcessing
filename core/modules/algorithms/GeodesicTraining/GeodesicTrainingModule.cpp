#include "GeodesicTrainingModule.h"

#include <QDebug>
#include <QMessageBox>

#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkIOUtil.h>
#include <mitkImageCast.h>
#include <mitkConvert2Dto3DImageFilter.h>

#include <itkExtractImageFilter.h>

#include <vector>
#include <string>
#include <mutex>

#include "GeodesicTrainingQt.h"
#include "CustomMitkDataStorage.h"

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
    CustomMitkDataStorage* ds = CustomMitkDataStorage::GetInstance();

    int numberOfImages = 0, numberOfSeedsImages = 0;

    auto iids = dm->GetAllDataIdsOfSubject(m_Uid);

    std::vector<long> inputImageIDs;
    long              seedsID;

    // Find the number of normal images and seeds
    for (const long& iid : iids)
	{
		QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid);
        QString dataType = m_DataManager->GetDataType(iid);

		if (dataSpecialRole == "Seeds") {
			numberOfSeedsImages++;
            seedsID = iid;
		}
		else if (dataSpecialRole == "" && dataType == "Image") {
			numberOfImages++;
            inputImageIDs.push_back(iid);
		}
	}

    // Abort if the numbers of images and seeds are not correct 
	if (numberOfSeedsImages == 0)
	{
        emit ProgressUpdateUI(m_Uid, "Error", -1);
		emit AlgorithmFinishedWithError(this, "No seeds drawn");
		return;
	}
	if (numberOfSeedsImages > 1)
	{
        emit ProgressUpdateUI(m_Uid, "Error", -1);
        emit AlgorithmFinishedWithError(this, "Multiple seeds. " +
            QString("Please remove all but one seeds ") +
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

    // Directory to save the result
    QString outputPath = dm->GetSubjectPath(m_Uid) + "/" + "GeodesicTraining";

    qDebug() << "GeodesicTraining will use " << m_IdealNumberOfThreads << " threads";
    
    // Load all the images as mitk::Image(s)
    std::vector< mitk::Image::Pointer > inputImagesMITK;
    for (const long& iid : inputImageIDs)
    {
        try {
            inputImagesMITK.push_back(
                ds->GetImage(iid)
            );
        } catch (...) {
            emit AlgorithmFinishedWithError(this, "Can't read image(s)");
            return;
        }
    }

    // Load the seeds as mitk::Image
    mitk::LabelSetImage::Pointer seedsMITK;
    try {
        seedsMITK = ds->GetLabelSetImage(seedsID);
    } catch (...) {
        emit AlgorithmFinishedWithError(this, "Can't read seeds image(s)");
        return;
    }

    // Find the image dimensions
    //const unsigned int dimensions = mitk::IOUtil::Load<mitk::Image>(images[0])->GetDimension();
    const unsigned int dimensions = inputImagesMITK[0]->GetDimension();
    qDebug() << "Dimensions are" << dimensions;

    // This necessary because of templates (and maybe because there is not a base class)
    if (dimensions == 2)
    {
        // 2D
        GeodesicTrainingQt<2>* geodesicTraining = new GeodesicTrainingQt<2>(this);

        connect(geodesicTraining, SIGNAL(GeodesicTrainingProgressUpdate(QString, int)),
            this, SLOT(GeodesicTrainingProgressUpdateHandler(QString, int))
        );

        // Lock the edit mutex
        std::unique_lock<std::mutex> ul(*this->GetDataManager()->GetSubjectEditMutexPointer(m_Uid));
        
        // Convert the images to itk::Image
        std::vector<typename itk::Image<float, 2>::Pointer> inputImagesITK;

        for (const mitk::Image::Pointer imageMITK : inputImagesMITK)
        {
            typename itk::Image<float, 2>::Pointer imageITK;
            mitk::CastToItkImage(imageMITK, imageITK);
            inputImagesITK.push_back(imageITK);
        }
        inputImagesMITK.clear();

        typename itk::Image<int, 2>::Pointer seedsITK;
        
        // Convert the seeds to 2D itk::Image (because mitk::LabelSetImage is always 3D)
        {
            typedef itk::Image<int, 2> LabelsImageType2D;
            typedef itk::Image<int, 3> LabelsImageType3D;
            typename LabelsImageType3D::Pointer seedsITK3D;
            mitk::CastToItkImage(seedsMITK, seedsITK3D);
            auto regionSize = seedsITK3D->GetLargestPossibleRegion().GetSize();
            regionSize[2] = 0; // Only 2D image is needed
            LabelsImageType3D::IndexType regionIndex;
            regionIndex.Fill(0);    
            LabelsImageType3D::RegionType desiredRegion(regionIndex, regionSize);
            auto extractor = itk::ExtractImageFilter< LabelsImageType3D, LabelsImageType2D >::New();
            extractor->SetExtractionRegion(desiredRegion);
            extractor->SetInput(seedsITK3D);
            extractor->SetDirectionCollapseToIdentity();
            extractor->Update();
            seedsITK = extractor->GetOutput();
            seedsITK->DisconnectPipeline();
        }

        // Initialize geodesicTraining
        geodesicTraining->SetInputImages(inputImagesITK);
        geodesicTraining->SetLabels(seedsITK);
        geodesicTraining->SetOutputPath(outputPath.toStdString());
        geodesicTraining->SetNumberOfThreads(m_IdealNumberOfThreads);
        geodesicTraining->SaveOnlyNormalSegmentation(true, "segmentation");
        geodesicTraining->SetVerbose(true);

        // Unlock edit mutex and run
        // TODO: maybe unlock after run?
        ul.unlock();
        auto result = geodesicTraining->Execute();

        if (result->ok)
        {
            mitk::Image::Pointer segmNormal;
            mitk::CastToMitkImage(result->labelsImage, segmNormal);
            mitk::Convert2Dto3DImageFilter::Pointer filter = mitk::Convert2Dto3DImageFilter::New();
            filter->SetInput(segmNormal);
            filter->Update();
            segmNormal = filter->GetOutput();

            mitk::LabelSetImage::Pointer segm = mitk::LabelSetImage::New();
            
            try {
                // Initialize the LabelSetImage with the output from the GeodesicTraining
                segm->InitializeByLabeledImage(segmNormal);

                // Copy the labels from seeds image
                {
                    mitk::LabelSet::Pointer referenceLabelSet =	
                        //mitk::IOUtil::Load<mitk::LabelSetImage>(seeds)->GetActiveLabelSet();
                        seedsMITK->GetActiveLabelSet();
                        // seedsMITK->GetLabelSet();

                    mitk::LabelSet::Pointer outputLabelSet = segm->GetActiveLabelSet();
                        //segm->GetLabelSet();

                    mitk::LabelSet::LabelContainerConstIteratorType itR;
                    mitk::LabelSet::LabelContainerConstIteratorType it;
                    
                    for (itR =  referenceLabelSet->IteratorConstBegin();
                            itR != referenceLabelSet->IteratorConstEnd(); 
                            ++itR) 
                    {
                        for (it = outputLabelSet->IteratorConstBegin(); 
                                it != outputLabelSet->IteratorConstEnd();
                                ++it)
                        {
                            if (itR->second->GetValue() == it->second->GetValue())
                            {
                                it->second->SetColor(itR->second->GetColor());
                                it->second->SetName(itR->second->GetName());
                            }
                        }
                    }
                }

                // TODO: Fix this hack (probably needs interactors or something)
                QString tmpPath = m_DataManager->GetSubjectPath(this->GetUid()) + "tmpGT.nrrd";
                mitk::IOUtil::Save(segm, tmpPath.toStdString());
                segm = mitk::IOUtil::Load<mitk::LabelSetImage>(tmpPath.toStdString());
                // EO Fix this hack

                segm->GetActiveLabelSet()->SetActiveLabel(0);
                                
                ds-> AddMitkLabelSetImageToSubject(this->GetUid(), 
                    segm, outputPath + "segmentation.nrrd", "Segmentation", "<Segmentation>   " 
                );
            } 
            catch (mitk::Exception& e) {
                MITK_ERROR << "Exception caught: " << e.GetDescription();
                emit AlgorithmFinishedWithError(this, "Could not initialize output segmentation image.");
                return;
            }

            qDebug() << "GeodesicTraining: Saved segmentation";

            // this->GetDataManager()->AddDataToSubject(this->GetUid(), 
            //     outputPath + QString("/segmentation.nrrd"), "Segmentation", 
            //     "Image", "<Segmentation>"
            // );
        }
        else {
            qDebug() << "GeodesicTraining finished with internal error";
            emit ProgressUpdateUI(m_Uid, "Error", -1);
            emit AlgorithmFinishedWithError(this, result->errorMessage.c_str());
        }

        delete geodesicTraining;
    }
    else {
        // 3D
        GeodesicTrainingQt<3>* geodesicTraining = new GeodesicTrainingQt<3>(this);

        connect(geodesicTraining, SIGNAL(GeodesicTrainingProgressUpdate(QString, int)),
            this, SLOT(GeodesicTrainingProgressUpdateHandler(QString, int))
        );

        std::unique_lock<std::mutex> ul(*this->GetDataManager()->GetSubjectEditMutexPointer(m_Uid));
        
        // Convert the images/seeds to itk::Image
        std::vector<typename itk::Image<float, 3>::Pointer> inputImagesITK;
        typename itk::Image<int, 3>::Pointer seedsITK;

        for (const mitk::Image::Pointer imageMITK : inputImagesMITK)
        {
            typename itk::Image<float, 3>::Pointer imageITK;
            mitk::CastToItkImage(imageMITK, imageITK);
            inputImagesITK.push_back(imageITK);
        }
        mitk::CastToItkImage(seedsMITK, seedsITK);

        geodesicTraining->SetInputImages(inputImagesITK);
        geodesicTraining->SetLabels(seedsITK);

        geodesicTraining->SetOutputPath(outputPath.toStdString());
        geodesicTraining->SetNumberOfThreads(m_IdealNumberOfThreads);
        //geodesicTraining->SaveOnlyNormalSegmentation(true, "segmentation");
        geodesicTraining->SetVerbose(true);
        
        ul.unlock();
        auto result = geodesicTraining->Execute();

        if (result->ok)
        {
            mitk::Image::Pointer segmNormal;
            mitk::CastToMitkImage(result->labelsImage, segmNormal);

            mitk::LabelSetImage::Pointer segm = mitk::LabelSetImage::New();
            
            try {
                // Initialize the LabelSetImage with the output from the GeodesicTraining
                segm->InitializeByLabeledImage(segmNormal);

                // Copy the labels from seeds image
                {
                    mitk::LabelSet::Pointer referenceLabelSet =	
                        //mitk::IOUtil::Load<mitk::LabelSetImage>(seeds)->GetActiveLabelSet();
                        seedsMITK->GetActiveLabelSet();
                        // seedsMITK->GetLabelSet();

                    mitk::LabelSet::Pointer outputLabelSet = segm->GetActiveLabelSet();
                        // segm->GetLabelSet();

                    mitk::LabelSet::LabelContainerConstIteratorType itR;
                    mitk::LabelSet::LabelContainerConstIteratorType it;
                    
                    for (itR =  referenceLabelSet->IteratorConstBegin();
                            itR != referenceLabelSet->IteratorConstEnd(); 
                            ++itR) 
                    {
                        for (it = outputLabelSet->IteratorConstBegin(); 
                                it != outputLabelSet->IteratorConstEnd();
                                ++it)
                        {
                            if (itR->second->GetValue() == it->second->GetValue())
                            {
                                it->second->SetColor(itR->second->GetColor());
                                it->second->SetName(itR->second->GetName());
                            }
                        }
                    }
                }

                // TODO: Fix this hack (probably needs interactors or something)
                QString tmpPath = m_DataManager->GetSubjectPath(this->GetUid()) + "tmpGT.nrrd";
                mitk::IOUtil::Save(segm, tmpPath.toStdString());
                segm = mitk::IOUtil::Load<mitk::LabelSetImage>(tmpPath.toStdString());
                // EO Fix this hack

                segm->GetActiveLabelSet()->SetActiveLabel(0);
                
                ds-> AddMitkLabelSetImageToSubject(this->GetUid(), 
                    segm, outputPath + "segmentation.nrrd", "Segmentation", "<Segmentation>   " 
                );
            } 
            catch (mitk::Exception& e) {
                MITK_ERROR << "Exception caught: " << e.GetDescription();
                emit AlgorithmFinishedWithError(this, "Could not initialize output segmentation image.");
                return;
            }

            qDebug() << "GeodesicTraining: Saved segmentation";

            // this->GetDataManager()->AddDataToSubject(this->GetUid(), 
            //     outputPath + QString("/segmentation.nrrd"), "Segmentation", 
            //     "Image", "<Segmentation>"
            // );
        }
        else {
            qDebug() << "GeodesicTraining finished with internal error";
            emit ProgressUpdateUI(m_Uid, "Error", -1);
            emit AlgorithmFinishedWithError(this, result->errorMessage.c_str());
        }

        delete geodesicTraining;
    }
}