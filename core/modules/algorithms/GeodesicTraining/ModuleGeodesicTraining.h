#ifndef MODULE_GEODESIC_TRAINING_SEGMENTATION_H
#define MODULE_GEODESIC_TRAINING_SEGMENTATION_H

#include "AlgorithmModuleBase.h"

class GeodesicTrainingModule : public AlgorithmModuleBase
{
	Q_OBJECT

public:
	explicit GeodesicTrainingModule(QObject *parent = nullptr);
	~GeodesicTrainingModule() {}

public slots:
	void GeodesicTrainingProgressUpdateHandler(QString message, int progress);
private:
	void Algorithm() override;
};

#endif // ! MODULE_GEODESIC_TRAINING_SEGMENTATION_H
