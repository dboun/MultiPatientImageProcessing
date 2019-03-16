#ifndef GEODESIC_TRAINING_QT_ONLY_H
#define GEODESIC_TRAINING_QT_ONLY_H

#include <QObject>

/** This class exists because you can't mix templates and QObject */
class GeodesicTrainingQtOnly : public QObject
{
    Q_OBJECT

public:
    explicit GeodesicTrainingQtOnly(QObject* parent = nullptr) : QObject(parent) {}
	virtual ~GeodesicTrainingQtOnly() {}

signals:
    void GeodesicTrainingProgressUpdate(QString message, int progress);
};

#endif // ! GEODESIC_TRAINING_QT_ONLY_H