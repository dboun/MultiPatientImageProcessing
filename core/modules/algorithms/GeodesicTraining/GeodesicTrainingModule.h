#ifndef MODULE_GEODESIC_TRAINING_SEGMENTATION_H
#define MODULE_GEODESIC_TRAINING_SEGMENTATION_H

#include <QThread>

#include "AlgorithmModuleBase.h"

class GeodesicTrainingModule : public AlgorithmModuleBase
{
	Q_OBJECT

public:
	GeodesicTrainingModule(QObject *parent = nullptr);
	~GeodesicTrainingModule();

public slots:
	void GeodesicTrainingProgressUpdateHandler(QString message, int progress);
private:
	void Algorithm() override;

	int m_IdealNumberOfThreads = (
		(QThread::idealThreadCount() > 4) ?
		QThread::idealThreadCount() - 1 :
		QThread::idealThreadCount()
	);

 //	int m_IdealNumberOfThreads = (
 //       (QThread::idealThreadCount() / 2 > 2) ? 
 //           QThread::idealThreadCount() / 2 : 
 //           2
 //   );

};

#endif // ! MODULE_GEODESIC_TRAINING_SEGMENTATION_H
