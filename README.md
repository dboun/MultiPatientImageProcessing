# MPIP (*Multi-Patient Image Processing*)

Bulk processing of medical images.

*If* GeodesicTrainingSegmentation crashes, build OpenCV with CPU_BASELINE set to 'SSE'. Alternatively, you can also update OpenCV to version 4 ([default of OpenCV now](https://github.com/opencv/opencv)). 

### Input data

Every individual subject's images should be in a folder. You can drag and drop one or more folders (for instance if there are X subjects drag and drop X folders; one for each subject). 

It's ok if the folders contain subfolders, but images inside the subfolders will be considered to be part of the same subject.

You can also use ```File>Import single subject``` to select a folder that contains images for only one subject. 
