#ifndef MITK_SEGMENTATION_TOOL_CONTROLLER_H
#define MITK_SEGMENTATION_TOOL_CONTROLLER_H

#include "MitkSegmentationTool.h"

class MitkSegmentationToolController
{
public:
    MitkSegmentationToolController(MitkSegmentationTool* mitkSegmentationTool);
    ~MitkSegmentationToolController() {}

public slots:
    void OnCreateNewLabelSetImageClicked();
	void OnAddNewLabelClicked();
};

#endif // ! MITK_SEGMENTATION_TOOL_CONTROLLER_H