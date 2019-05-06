#ifndef H_CBICA_PROCESSING
#define H_CBICA_PROCESSING

#include <vector>

#include "cbicaITKSafeImageIO.h"
#include "cbicaITKImageInfo.h"

#include "UtilItkGTS.h"
#include "SvmSuite.h"

namespace GeodesicTrainingSegmentation
{
	/** This class handles preprocessing and postprocessing operations */
	template<typename PixelType, unsigned int Dimensions>
	class Processing
	{
	public:
		typedef int   AgdPixelType;
		typedef int   LabelsPixelType;
		typedef float PseudoProbPixelType;

		typedef itk::Image< AgdPixelType, Dimensions >        AgdImageType;
		typedef itk::Image< PixelType, Dimensions >           InputImageType;
		typedef itk::Image< LabelsPixelType, Dimensions >     LabelsImageType;
		typedef itk::Image< PseudoProbPixelType, Dimensions > PseudoProbImageType;

		typedef typename AgdImageType::Pointer                AgdImagePointer;
		typedef typename InputImageType::Pointer              InputImagePointer;
		typedef typename LabelsImageType::Pointer             LabelsImagePointer;
		typedef typename PseudoProbImageType::Pointer         PseudoProbImagePointer;

		void SetLimitPixels(bool limitPixels, int pixelLimit = 10000000)
		{
			m_limit_pixels = limitPixels;
			m_pixel_limit  = pixelLimit;
		}

		void SetDoStatisticalNormalization(bool doStatisticalNormalization, float imageToAgdMapsRatio = 6)
		{
			m_do_statistical_normalization = doStatisticalNormalization;
			m_image_to_agd_maps_ratio = imageToAgdMapsRatio;
		}

		void SetDoCurvatureAnisotropic(bool doCurvatureAnisotropic)
		{
			m_do_curvature_anisotropic = doCurvatureAnisotropic;
		}

		void SetVerbose(bool verbose)
		{
			m_verbose = verbose;
		}

		void SetSaveAll(bool saveAll)
		{
			m_save_all = saveAll;
		}

		void SetOutputFolder(std::string outputFolder)
		{
			m_output_folder = outputFolder;
		}

		void SetTimerEnabled(bool timerEnabled) 
		{
			m_timer_enabled = timerEnabled;
		}

		void PreProcess(std::vector<InputImagePointer>& inputImages, LabelsImagePointer& labels)
		{
			if (m_verbose) { std::cout << "Preprocessing:\n"; }

			m_original_input_image_size     = inputImages[0]->GetLargestPossibleRegion().GetSize();
			m_original_labels_image_size    = labels->GetLargestPossibleRegion().GetSize();
			m_original_input_image_spacing  = inputImages[0]->GetSpacing();
			m_original_labels_image_spacing = labels->GetSpacing();

			if (m_verbose)
			{
				std::cout << "\tImage size:     " << m_original_input_image_size << "\n";
				std::cout << "\tLabels size:    " << m_original_labels_image_size << "\n";
				std::cout << "\tImage spacing:  " << m_original_input_image_spacing << "\n";
				std::cout << "\tLabels spacing: " << m_original_labels_image_spacing << "\n";
			}

			/* ------ Find out if spacing should be normalized ------ */

			// Find minimum, maximum and average spacing
			auto   minSpacing = m_original_input_image_spacing[0];
			auto   maxSpacing = m_original_input_image_spacing[0];
			double avgSpacing = m_original_input_image_spacing[0];
			for(int i=1; i<InputImageType::ImageDimension; i++)
			{
				if (m_original_input_image_spacing[i] > maxSpacing) {
					maxSpacing = m_original_input_image_spacing[i];
				} 
				if (m_original_input_image_spacing[i] < minSpacing) {
					minSpacing = m_original_input_image_spacing[i];
				} 
				avgSpacing += m_original_input_image_spacing[i];
			}
			avgSpacing /= InputImageType::ImageDimension;

			// Find if spacing should be normalized (i.e. there is a big difference across dimensions)
			bool shouldNormalizeSpacing = false;
			for(int i=0; i<InputImageType::ImageDimension; i++)
			{
				auto dimSpacingDiff = m_original_input_image_spacing[i] - minSpacing;
				if (dimSpacingDiff > 0.1 || dimSpacingDiff < -0.1)
				{
					shouldNormalizeSpacing = true;
				}
			}

			// This variable holds the normal image size if spacing will not be changed,
			// or it will hold the value of the new size that will result from changing the spacing.
			typename InputImageType::SizeType tSize = m_original_input_image_size;

			// Find out if size should be normalized (i.e. the pixel count is big)
			bool shouldNormalizeSize = false;
			if (m_limit_pixels) 
			{
				if (shouldNormalizeSpacing)
				{
					// Find how the size would look if we (theoretically) just normalized the spacing
					for (unsigned int i=0; i < InputImageType::ImageDimension; i++) {
						tSize[i] = m_original_input_image_size[i] * (
							static_cast<double>(m_original_input_image_spacing[i]) / minSpacing
						);
					}
				}

				int  pixelCount = 1;
				for(int i=0; i<InputImageType::ImageDimension; i++)
				{
					pixelCount *= tSize[i];
				}
				if (pixelCount > m_pixel_limit) 
				{ 
					shouldNormalizeSize = true; 
				}
			}

			// ------ Different size/spacing operations (and nothing when both booleans are false) ------
			startTimer();
			if (shouldNormalizeSize && !shouldNormalizeSpacing)
			{
				// Just normalize the size to something that has about m_pixel_limit pixels.
				// If the image has less, the filter does nothing.
				if (m_verbose) { std::cout << "\tImages are too big and will be subsampled\n"; }
				for (auto& image : inputImages) {
					image = ItkUtilGTS::resizeImageMaximumPixelNumber<InputImageType>(image, m_pixel_limit);
				}
				labels = ItkUtilGTS::resizeImageMaximumPixelNumber<LabelsImageType>(labels, m_pixel_limit, true);
			}
			else if (!shouldNormalizeSize && shouldNormalizeSpacing)
			{
				// Just normalize the spacing
				if (m_verbose) { std::cout << "\nSpacing should be corrected\n"; }
				for (auto& image : inputImages) {
					typename InputImageType::SpacingType sp;
					sp.Fill(minSpacing);
					image = ItkUtilGTS::changeImageSpacing<InputImageType>(image, sp);
				}
				typename LabelsImageType::SpacingType sp;
				sp.Fill(minSpacing);
				labels = ItkUtilGTS::changeImageSpacing<LabelsImageType>(labels, sp, true);
			}
			else if (shouldNormalizeSize && shouldNormalizeSpacing)
			{
				// Just normalizing the spacing to average is not enough because it will result in a big image
				// We just need a higher value for the spacing that brings the size into the limits
				// For instance if pixel count is 9 times bigger than the pixel limit and the image is 3D
				// then we increase the spacing by sp * (9)^(1/3) in each dimension
				if (m_verbose) { std::cout << "\tBoth size and spacing should be adjusted.\n"; }

				// How many pixels would the resulting image have if 
				// just changed the spacing is changed (tSize is calculated above)
				int  pixelCount = 1;
				for(int i=0; i<InputImageType::ImageDimension; i++)
				{
					pixelCount *= tSize[i];
				}

				double ratio = 1.0 * pixelCount / m_pixel_limit;
				for (auto& image : inputImages) {
					typename InputImageType::SpacingType limSp;
					limSp.Fill(minSpacing * std::pow(ratio, 1.0/InputImageType::ImageDimension));
					image = ItkUtilGTS::changeImageSpacing<InputImageType>(image, limSp);
				}
				typename InputImageType::SpacingType limSp;
				limSp.Fill(minSpacing * std::pow(ratio, 1.0/InputImageType::ImageDimension));
				labels = ItkUtilGTS::changeImageSpacing<LabelsImageType>(labels, limSp, true);
			}
			stopTimerAndReport("Preprocessing: Normalizing image size and spacing");

			// Print image information if anything changed
			if ((shouldNormalizeSize || shouldNormalizeSpacing) && m_verbose)
			{
				std::cout << "\t After normalizing spacing and size:\n";
				std::cout << "\t\t Size:    " << labels->GetLargestPossibleRegion().GetSize() << "\n";
				std::cout << "\t\t Spacing: " << labels->GetSpacing() << "\n";
			}

			// Save the intermediate images if saveall is set
			if (m_save_all)
			{
				for (auto& image : inputImages) 
				{
					static int y = 0;
					cbica::WriteImage<InputImageType>( image, m_output_folder + "/" +
						std::string("pre_image_pre_size_origin") + std::to_string(++y) + ".nii.gz"
					);
				}		
				cbica::WriteImage<LabelsImageType>( labels, 
					m_output_folder + std::string("/") + "pre_labels_size_origin.nii.gz"
				);
			}

			// ------ Normal filters after size/spacing is settled ------

			if (m_do_statistical_normalization)
			{
				startTimer();
				if (m_verbose) { std::cout << "\tNormalizing images\n"; }
				ItkUtilGTS::statisticalImageVectorNormalization<InputImageType>(
					inputImages, 
					std::lround(255 * m_image_to_agd_maps_ratio)
				);
				stopTimerAndReport("Preprocessing: Statistical image normalization");

				if (m_save_all)
				{				
					for (auto& image : inputImages)
					{
						static int w = 0;
						cbica::WriteImage<InputImageType>( image, m_output_folder + "/" +
							std::string("statistical_norm_image") + std::to_string(++w) + std::string(".nii.gz")
						);
					}
				}
			}

			if (m_do_curvature_anisotropic)
			{
				startTimer();
				if (m_verbose) { std::cout << "\tFiltering images\n"; }
				for (auto& image : inputImages) {
					image = ItkUtilGTS::curvatureAnisotropicDiffusionImageFilter<InputImageType>(image);
					if (m_save_all)
					{				
						static int z = 0;
						cbica::WriteImage<InputImageType>( image, m_output_folder + "/" +
							std::string("cadf_image") + std::to_string(++z) + std::string(".nii.gz")
						);
					}
				}
				stopTimerAndReport("Preprocessing: Curvature Anisotropic Diffusion Image Filter");
			}
		}

		void PostProcessLabelsImage(LabelsImagePointer& labels)
		{
			// Make the output the same as the original input
			if (m_verbose) { std::cout << "Postprocessing labels image\n"; }

			// if (m_limit_pixels)
			// {
			// 	labels = ItkUtilGTS::resizeImage<LabelsImageType>(labels, m_original_labels_image_size, true);
			// }

			startTimer();
			labels = ItkUtilGTS::changeImageSpacing<LabelsImageType>(
				labels, m_original_labels_image_spacing, true, 
				true, m_original_labels_image_size
			);
			stopTimerAndReport("Postprocessing: Changing size and spacing back");
			if (m_save_all)
			{
				cbica::WriteImage<LabelsImageType>( labels, m_output_folder + "/" +
					std::string("labels_before_post_processing.nii.gz")
				);
			}	
		}

		template <class TImageType>
		void PostProcessNormalImages(std::vector<typename TImageType::Pointer>& images)
		{
			for (auto& image : images) {
				image = ItkUtilGTS::changeImageSpacing<TImageType>(
					image, m_original_input_image_spacing, false, 
					true, m_original_input_image_size
				);	
			}
		}

		template <class TImageType>
		void PostProcessNormalImage(typename TImageType::Pointer& image)
		{
			if (m_verbose) { std::cout << "Postprocessing image\n"; }
			startTimer();
			std::vector<typename TImageType::Pointer> imageInVector;
			imageInVector.push_back(image);
			PostProcessNormalImages<TImageType>(imageInVector);
			stopTimerAndReport("Postprocessing: Changing a non-label image's size and spacing back");
		}

	private:
		typename InputImageType::SpacingType  m_original_input_image_spacing;
		typename LabelsImageType::SpacingType m_original_labels_image_spacing;

		typename InputImageType::SizeType  m_original_input_image_size;
		typename LabelsImageType::SizeType m_original_labels_image_size;

		bool  m_limit_pixels = true, m_do_curvature_anisotropic = true, m_do_statistical_normalization = true,
		      m_verbose = false, m_save_all = false, m_timer_enabled = false;
		int   m_pixel_limit  = 5000000;
		float m_image_to_agd_maps_ratio = 6;
		std::string m_output_folder = cbica::getExecutablePath();
		SvmSuiteUtil::Timer m_timer;

		void startTimer() {
			if (m_timer_enabled) {
				m_timer.Reset();
			}
		}

		void stopTimerAndReport(std::string desc) {
			if (m_timer_enabled) {
				float diff = m_timer.Diff();

				std::ofstream timerFile;
				timerFile.open(m_output_folder + "/time_report.txt", std::ios_base::app); //append file
				timerFile << desc << ": " << diff << "s\n";
			}
		}

	};
}

#endif // ! H_CBICA_PROCESSING