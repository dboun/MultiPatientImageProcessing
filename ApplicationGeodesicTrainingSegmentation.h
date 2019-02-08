#ifndef APPLICATION_GEODESIC_TRAINING_SEGMENTATION_H
#define APPLICATION_GEODESIC_TRAINING_SEGMENTATION_H

#include "ApplicationBase.h"
#include "GeodesicTrainingSegmentation.h"

#include <QDebug>
#include <QString>

template<class PixelType, unsigned long Dimensions>
class ApplicationGeodesicTrainingSegmentation : public GeodesicTrainingSegmentation::Coordinator<PixelType, Dimensions>, 
                                                public ApplicationBase
{
public:
	/** Progress update method overriden from GeodesicTrainingSegmentation */
	void progressUpdate(std::string message, int progress) override
	{
		qDebug() << QString("MESSAGE!!!: ") << QString::fromStdString(message) << QString(progress);

		if (message == "GTS: Executing" && progress == 0)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 0);
		}
		else if (message == "GTS: AGD Operations" && progress == 0)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 5);
		}
		else if (message == "GTS: AGD Operations" && progress == 100)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 20);
		}
		else if (message == "GTS: Converting" && progress == 0)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 25);
		}
		else if (message == "GTS: Converting" && progress == 100)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 30);
		}
		else if (message == "GTS: SVM Operations" && progress == 0)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 35);
		}
		else if (message == "GTS: SVM Operations" && progress == 100)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 99);
		}
		else if (message == "GTS: Finished" && progress == 0)
		{
			emit ProgressUpdateUI(uid, QString::fromStdString(message), 100);
		}
	}
};

#endif // APPLICATION_GEODESIC_TRAINING_SEGMENTATION_H