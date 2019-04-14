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

    // Find the number of images and masks
    for (const long& iid : iids)
	{
		QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid); 
        QString dataPath = m_DataManager->GetDataPath(iid);

		if (dataSpecialRole == "Mask"/* && dataPath.endsWith(".nii.gz", Qt::CaseSensitive)*/) {
			numberOfMasks++;
            //qDebug() << "Found nifti mask, with path" << dataPath;
            mask = dataPath.toStdString();
		}
		else if (dataSpecialRole == ""/* && dataPath.endsWith(".nii.gz", Qt::CaseSensitive)*/)
		{
			numberOfImages++;
            images.push_back(dm->GetDataPath(iid).toStdString());
		}
	}

    // Abort if the numbers of images and masks are not correct 
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

    // Directory to save the result
    QString outputPath = dm->GetSubjectPath(m_Uid) + 
        "/" + this->GetAppNameShort() +
        "/" + this->GetAppNameShort() + "_Segmentation";

    qDebug() << "GeodesicTraining will use " << m_IdealNumberOfThreads << " threads";
    qDebug() << "GeodesicTraining will use mask " << mask.c_str();

    // Load all the images as mitk::Image(s)
    std::vector< mitk::Image::Pointer > inputImagesMITK;
    for (const std::string& image : images)
    {
        inputImagesMITK.push_back(
            mitk::IOUtil::Load<mitk::Image>(image)
        );
    }
    // Load the mask as mitk::Image
    mitk::Image::Pointer maskMITK = mitk::IOUtil::Load<mitk::Image>(mask);

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

        typename itk::Image<int, 2>::Pointer maskITK;
        
        // Convert the mask to 2D itk::Image (because mitk::LabelSetImage is always 3D)
        {
            typedef itk::Image<int, 2> LabelsImageType2D;
            typedef itk::Image<int, 3> LabelsImageType3D;
            typename LabelsImageType3D::Pointer maskITK3D;
            mitk::CastToItkImage(maskMITK, maskITK3D);
            auto regionSize = maskITK3D->GetLargestPossibleRegion().GetSize();
            regionSize[2] = 0; // Only 2D image is needed
            LabelsImageType3D::IndexType regionIndex;
            regionIndex.Fill(0);    
            LabelsImageType3D::RegionType desiredRegion(regionIndex, regionSize);
            auto extractor = itk::ExtractImageFilter< LabelsImageType3D, LabelsImageType2D >::New();
            extractor->SetExtractionRegion(desiredRegion);
            extractor->SetInput(maskITK3D);
            extractor->SetDirectionCollapseToIdentity();
            extractor->Update();
            maskITK = extractor->GetOutput();
            maskITK->DisconnectPipeline();
        }

        // Initialize geodesicTraining
        geodesicTraining->SetInputImages(inputImagesITK);
        geodesicTraining->SetLabels(maskITK);
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

                // Copy the labels from mask image
                {
                    mitk::LabelSet::Pointer referenceLabelSet =	
                        mitk::IOUtil::Load<mitk::LabelSetImage>(mask)->GetActiveLabelSet();
                    
                    mitk::LabelSet::Pointer outputLabelSet = segm->GetActiveLabelSet();

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
                
                // Save to file
                mitk::IOUtil::Save(
		            segm, 
		            outputPath.toStdString() + std::string("/segmentation.nrrd")
	            );
            } 
            catch (mitk::Exception& e) {
                MITK_ERROR << "Exception caught: " << e.GetDescription();
                emit AlgorithmFinishedWithError(this, "Could not initialize output segmentation image.");
                return;
            }

            qDebug() << "GeodesicTraining: Saved segmentation";

            this->GetDataManager()->AddDataToSubject(this->GetUid(), 
                outputPath + QString("/segmentation.nrrd"), "Segmentation"
            );
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
        
        // Convert the images/mask to itk::Image
        std::vector<typename itk::Image<float, 3>::Pointer> inputImagesITK;
        typename itk::Image<int, 3>::Pointer maskITK;

        for (const mitk::Image::Pointer imageMITK : inputImagesMITK)
        {
            typename itk::Image<float, 3>::Pointer imageITK;
            mitk::CastToItkImage(imageMITK, imageITK);
            inputImagesITK.push_back(imageITK);
        }
        mitk::CastToItkImage(maskMITK, maskITK);

        geodesicTraining->SetInputImages(inputImagesITK);
        geodesicTraining->SetLabels(maskITK);

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

                // Copy the labels from mask image
                {
                    mitk::LabelSet::Pointer referenceLabelSet =	
                        mitk::IOUtil::Load<mitk::LabelSetImage>(mask)->GetActiveLabelSet();
                    
                    mitk::LabelSet::Pointer outputLabelSet = segm->GetActiveLabelSet();

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
                
                // Save to file
                mitk::IOUtil::Save(
		            segm, 
		            outputPath.toStdString() + std::string("/segmentation.nrrd")
	            );
            } 
            catch (mitk::Exception& e) {
                MITK_ERROR << "Exception caught: " << e.GetDescription();
                emit AlgorithmFinishedWithError(this, "Could not initialize output segmentation image.");
                return;
            }

            qDebug() << "GeodesicTraining: Saved segmentation";

            this->GetDataManager()->AddDataToSubject(this->GetUid(), 
                outputPath + QString("/segmentation.nrrd"), "Segmentation"
            );
        }
        else {
            qDebug() << "GeodesicTraining finished with internal error";
            emit ProgressUpdateUI(m_Uid, "Error", -1);
            emit AlgorithmFinishedWithError(this, result->errorMessage.c_str());
        }

        delete geodesicTraining;
    }
}