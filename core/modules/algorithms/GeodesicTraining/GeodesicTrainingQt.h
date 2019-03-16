#ifndef GEODESIC_TRAINING_QT_H
#define GEODESIC_TRAINING_QT_H

#include "GeodesicTrainingQtOnly.h"
#include "GeodesicTrainingSegmentation.h"

template<unsigned long Dimensions>
class GeodesicTrainingQt : public GeodesicTrainingQtOnly,
	public GeodesicTrainingSegmentation::Coordinator<float, Dimensions>
{
public:
	explicit GeodesicTrainingQt(QObject* parent = nullptr) : GeodesicTrainingQtOnly(parent) {}
	virtual ~GeodesicTrainingQt() {}

protected:
	/** Progress update method overriden from GeodesicTrainingSegmentation */
	void progressUpdate(std::string message, int progress) override
	{
		if (message == "GTS: Executing" && progress == 0)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 0);
		}
		else if (message == "GTS: AGD Operations" && progress == 0)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 5);
		}
		else if (message == "GTS: AGD Operations" && progress == 100)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 20);
		}
		else if (message == "GTS: Converting" && progress == 0)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 25);
		}
		else if (message == "GTS: Converting" && progress == 100)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 30);
		}
		else if (message == "GTS: SVM Operations" && progress == 0)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 35);
		}
		else if (message == "GTS: SVM Operations" && progress == 100)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 99);
		}
		else if (message == "GTS: Finished" && progress == 0)
		{
			emit GeodesicTrainingProgressUpdate(QString::fromStdString(message), 100);
		}
	}
};

#endif // ! GEODESIC_TRAINING_QT_H