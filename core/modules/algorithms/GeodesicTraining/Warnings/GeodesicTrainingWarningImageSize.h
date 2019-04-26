#ifndef GEODESIC_TRAINING_WARNING_IMAGE_SIZE_H
#define GEODESIC_TRAINING_WARNING_IMAGE_SIZE_H

#include "WarningFunctionBase.h"

class CustomMitkDataStorage;

class GeodesicTrainingWarningImageSize : public WarningFunctionBase
{
    Q_OBJECT

public:
    GeodesicTrainingWarningImageSize(QObject* parent = nullptr);

    ~GeodesicTrainingWarningImageSize();

    void SetDataManager(DataManager* dataManager) override;

    void SetDataView(DataViewBase* dataView) override;

public slots:
    // Slots for DataViewBase
	void SelectedSubjectChangedHandler(long uid) override;
	void DataAddedForSelectedSubjectHandler(long iid) override;
	void DataRemovedFromSelectedSubjectHandler(long iid) override;

private:
    CustomMitkDataStorage* m_DataStorage;

    void InspectImage(long iid);
};

#endif // ! GEODESIC_TRAINING_WARNING_IMAGE_SIZE_H