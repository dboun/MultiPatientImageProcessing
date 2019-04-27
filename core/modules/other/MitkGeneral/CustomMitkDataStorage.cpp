#include "CustomMitkDataStorage.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>

#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkIOUtil.h>
#include <mitkConvert2Dto3DImageFilter.h>
#include <mitkRenderingManager.h>

#include <stdexcept>
#include <string>

CustomMitkDataStorage* CustomMitkDataStorage::CreateInstance(DataManager* dataManager)
{
    m_DataManager = dataManager;
    m_CurrentSubjectID = -1;
    return GetInstance();
}

CustomMitkDataStorage* CustomMitkDataStorage::GetInstance()
{
    // static is initialized only once
    static CustomMitkDataStorage::Pointer instance = CustomMitkDataStorage::New(); 
    return instance;
}

CustomMitkDataStorage::~CustomMitkDataStorage()
{
    qDebug() << "CustomMitkDataStorage::~CustomMitkDataStorage()";
}

void CustomMitkDataStorage::SetAppNameShort(QString appNameShort)
{
	m_AppNameShort = appNameShort;
}

void CustomMitkDataStorage::SetDataView(DataViewBase* dataView)
{
    connect(dataView, SIGNAL(SelectedSubjectChanged(long)), 
        this, SLOT(SelectedSubjectChangedHandler(long))
    );
    connect(dataView, SIGNAL(SelectedDataChanged(long)), 
        this, SLOT(SelectedDataChangedHandler(long))
    );
	connect(dataView, SIGNAL(DataAddedForSelectedSubject(long)), 
        this, SLOT(DataAddedForSelectedSubjectHandler(long))
    );
	connect(dataView, SIGNAL(DataRemovedFromSelectedSubject(long)), 
        this, SLOT(DataRemovedFromSelectedSubjectHandler(long))
    );
	connect(dataView, SIGNAL(DataRequestedAsSeeds(long)), 
        this, SLOT(DataRequestedAsSeedsHandler(long))
    );
	connect(dataView, SIGNAL(DataRequestedAsSegmentation(long)), 
        this, SLOT(DataRequestedAsSegmentationHandler(long))
    );
	connect(dataView, SIGNAL(ExportData(long, QString)),
        this, SLOT(ExportDataHandler(long, QString))
    );
}

long CustomMitkDataStorage::AddMitkImageToSubject(long uid, 
    mitk::Image::Pointer mitkImage, QString path, 
    QString specialRole, QString name,
    bool external, bool visibleInDataView)
{
    qDebug() << "CustomMitkDataStorage::AddImageToSubject";

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitkImage);
    
    return this->AddMitkNodeToSubject(uid, node, 
        path, specialRole, "Image", name, external, visibleInDataView
    );
}

long CustomMitkDataStorage::AddMitkLabelSetImageToSubject(long uid, 
    mitk::LabelSetImage::Pointer mitkImage, QString path,
    QString specialRole, QString name,
    bool external, bool visibleInDataView)
{
    qDebug() << "CustomMitkDataStorage::AddLabelSetImageToSubject";
    
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitkImage);
    
    return this->AddMitkNodeToSubject(uid, node, 
        path, specialRole, "LabelSetImage", name, external, visibleInDataView
    );
}

long CustomMitkDataStorage::AddEmptyMitkLabelSetImageToSubject(long uid, 
    QString specialRole, QString name, bool external, bool visibleInDataView)
{
    if (uid == -1) { uid = m_CurrentSubjectID; }
    if (uid == -1) { return -1; }

    long referenceIid = -1;
    for (const long& iid : m_DataManager->GetAllDataIdsOfSubject(uid))
    {
        if (m_DataManager->GetDataType(iid) == "Image")
        {
            referenceIid = iid;
            break;
        }
    }

    bool isCurrentSubject = (uid == m_CurrentSubjectID);

    mitk::Image::Pointer referenceImage;

    if (isCurrentSubject && referenceIid != -1)
    {
        referenceImage = dynamic_cast<mitk::Image*>(
            this->GetNamedNode(std::to_string(referenceIid).c_str())->GetData()
        );
    }
    else if ((!isCurrentSubject) && referenceIid != -1)
    {
        referenceImage = mitk::IOUtil::Load<mitk::Image>(
            m_DataManager->GetDataPath(referenceIid).toStdString()
        );
    }
    else {
        // No normal image exists. Look for LabelSetImage.
        for (const long& iid : m_DataManager->GetAllDataIdsOfSubject(uid))
        {
            if (m_DataManager->GetDataType(iid) == "LabelSetImage")
            {
                referenceIid = iid;
                break;
            }
        }

        if (referenceIid == -1) { return -1; } // Return -1 if nothing more can be done

        // Even though it is a LabelSetImage at heart, it can be loaded as Image
        referenceImage = mitk::IOUtil::Load<mitk::Image>(
            m_DataManager->GetDataPath(referenceIid).toStdString()
        );
    }

    mitk::LabelSetImage::Pointer maskImage = mitk::LabelSetImage::New();
    try
    {
        maskImage->Initialize(referenceImage);
    }
    catch (mitk::Exception& e) {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        QMessageBox::information(nullptr, "New Segmentation Session", 
            "Could not create a new segmentation session.\n"
        );
        return -1;
    }

    // Create the directory to save if it doesn't exist
    QString directoryName = m_DataManager->GetSubjectPath(uid);

    if (!QDir(directoryName).exists())
    {
        QDir().mkpath(directoryName);
    }

    // Generate a random name (can't use iid, because it doesn't exist yet)
    // (also can't use name because it might not be unique)
    const QString possibleCharacters(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    );
    const int randomStringLength = 12;
    QString randomString;
    for(int i=0; i<randomStringLength; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    QString path = directoryName + "/" + randomString + ".nrrd";

    // Add the node to the wait list
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(maskImage);
    if (name == "") { name = "<" + specialRole + ">"; }
    m_NodesWaitMap[name] = dataNode;

    // mitk::IOUtil::Save(
    //     maskImage,
    //     nrrd.toStdString()
    // );

    return m_DataManager->AddDataToSubject(
        uid, path, specialRole, "LabelSetImage", name
    );
}

long CustomMitkDataStorage::ReAddAsLabelSetImage(long iid, QString newSpecialRole, 
    bool syncColors, long referenceIid)
{
    // --- Original image info ---
    QString originalName             = m_DataManager->GetDataName(iid);
    QString originalDataPath         = m_DataManager->GetDataPath(iid);
    QString originalType             = m_DataManager->GetDataType(iid);
    QString originalSpecialRole      = m_DataManager->GetDataSpecialRole(iid);
	bool originalIsExternal          = m_DataManager->GetDataIsExternal(iid);
	bool originalIsVisibleInDataView = m_DataManager->GetDataIsVisibleInDataView(iid);
	long uid                         = m_DataManager->GetSubjectIdFromDataId(iid);

    // --- New image info ---
    QString newName;
    {
        // Setting newName
        newName = originalName;
        QString originalSpecialRolePrefix = "<" + originalSpecialRole + ">";
        if (originalName.startsWith(originalSpecialRolePrefix))
        {
            newName.remove(0, originalSpecialRolePrefix.size());
            if (newSpecialRole == "" && newName.startsWith("( "))
            {
                // Remove the parentheses around the basename
                newName.remove(0, 2);
                newName.remove(newName.size()-1, 1);
            }
            else if (newSpecialRole == "" && newName == "")
            {
                newName = "LabelSetImage";
            }
            else if (newSpecialRole != "")
            {
                if (newName != "") { newName = " " + newName; }
                newName = "<" + newSpecialRole + ">" + newName;
            }
        }
        else if (newSpecialRole != "")
        {
            if (newName != "") { newName = " " + newName; }
            newName = "<" + newSpecialRole + "> (" + newName + ")";
        }
    }
    QString newDataPath;
    {
        // Setting newDataPath

        // Generate a random name (can't use newIid, because it doesn't exist yet)
        // (also can't use name because it might not be unique)
        const QString possibleCharacters(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        );
        const int randomStringLength = 12;
        QString randomString;
        for(int i=0; i<randomStringLength; ++i)
        {
            int index = qrand() % possibleCharacters.length();
            QChar nextChar = possibleCharacters.at(index);
            randomString.append(nextChar);
        }
        newDataPath = m_DataManager->GetSubjectPath(uid) + "/" + randomString + ".nrrd";
    }

    // --- Checks / operations without involving mitk ---
    if (originalType != "Image" && originalType != "LabelSetImage") // Also checks if iid = -1
    { 
        return -1; 
    }

    if (syncColors && m_DataManager->GetDataType(referenceIid) != "LabelSetImage") 
    { 
        referenceIid = -1; // Automatically pick
    }

    if (syncColors && referenceIid == -1)
    {
        // Automatically set the reference iid with priority to seeds
        if (m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Segmentation").size() > 0)
        {
            referenceIid = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
                uid, "Segmentation"
            )[0];
        }
        if (m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Seeds").size() > 0)
        {
            referenceIid = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
                uid, "Seeds"
            )[0];
        }

        if (referenceIid != -1) 
        {
            qDebug() << "Automatically picked" << referenceIid << "as reference iid";
        }
        else {
            qDebug() << "No image to set as reference iid automatically";
            syncColors = false;
        }
    }

    if (!syncColors 
        && originalType == "LabelSetImage"
        && originalSpecialRole == newSpecialRole) 
    { 
        qDebug() << "No changes can be made";
        return iid; 
    }

    if (!syncColors && uid != m_CurrentSubjectID && originalType == "LabelSetImage") 
    {
        // Just need to change the specialRole and potentially the name
        // LabelSetImage -> always internal

        // Add and remove need to happen in this order to not get the subject removed
        // if there is only this image in the subject
        long newIid = m_DataManager->AddDataToSubject(uid, newDataPath,
            newSpecialRole, "LabelSetImage", newName, 
            false, originalIsVisibleInDataView
        );
        m_DataManager->RemoveData(iid);
        return newIid;
    }

    // --- Actual conversions ---
    qDebug() << "Doing an actual conversion";
    mitk::LabelSetImage::Pointer newLSImage = mitk::LabelSetImage::New();

    // If the originalType is "Image" it needs to be converted
    if (originalType == "Image")
    {
        qDebug() << "Original type was Image";
        mitk::Image::Pointer originalImage;

        mitk::Image::Pointer originalImageFromDS;

        try {
            originalImageFromDS = this->GetImage(iid);
        } catch (...) {
            qDebug() << "Something went wrong with reading the original image";
            return -1;
        }

        if (!originalImageFromDS) {
            qDebug() << "Original image cannot be used";
            return -1;
        }

        // If the original image is 2D convert to 3D (because LabelSetImage needs to be 3D)
        qDebug() << "Checking image dimensions";
        if (originalImageFromDS->GetDimension() == 3)
        {
            qDebug() << "Image is 3D";
            originalImage = originalImageFromDS;
        }
        else if (originalImageFromDS->GetDimension() == 2)
        {
            qDebug() << "Image is 2D";
            mitk::Convert2Dto3DImageFilter::Pointer convertFilter = 
                mitk::Convert2Dto3DImageFilter::New();
            convertFilter->SetInput(originalImageFromDS);
            convertFilter->Update();
            originalImage= convertFilter->GetOutput();
        }
        else {
            return -1;
        }

        // Copy the data from input image
        try {
            newLSImage->InitializeByLabeledImage(originalImage);
        } catch (mitk::Exception e) {
            qDebug() << "Initializing didn't work";
            qDebug() << e.GetDescription();
            return -1;
        }
    }
    else {
        qDebug() << "Original type was LabelSetImage";
        newLSImage = this->GetLabelSetImage(iid);
    }

    // Sync colors (labels) from reference image if necessary
    if (syncColors && referenceIid != -1)
    {
        qDebug() << "Syncing colors";
        mitk::LabelSetImage::Pointer referenceImage = this->GetLabelSetImage(referenceIid);
        if (referenceImage && newLSImage)
        {
            for(unsigned int lidx = 0 ; lidx < newLSImage->GetNumberOfLayers(); lidx++)
            {
                mitk::LabelSet::Pointer referenceLabelSet =	//referenceImage->GetActiveLabelSet();
                    referenceImage->GetLabelSet(lidx);
                mitk::LabelSet::Pointer outputLabelSet    =	//newLSImage->GetActiveLabelSet();
                    newLSImage->GetLabelSet(lidx);

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
                            qDebug() << "Changing color";
                            it->second->SetColor(itR->second->GetColor());
                            it->second->SetName(itR->second->GetName());
                        }
                    }
                }
            }

            // if (newSpecialRole == "Segmentation") { newLSImage->GetActiveLabelSet()->SetActiveLabel(0); }
        }
    }

    // TODO: Fix this hack (probably needs interactors or something)
    QString tmpPath = m_DataManager->GetSubjectPath(uid) + "tmp.nrrd";
    mitk::IOUtil::Save(newLSImage, tmpPath.toStdString());
    newLSImage = mitk::IOUtil::Load<mitk::LabelSetImage>(tmpPath.toStdString());
    // EO Fix this hack

    if (newSpecialRole == "Segmentation") { newLSImage->GetActiveLabelSet()->SetActiveLabel(0); }

    // --- Updating DataManager & DataStorage ---
    qDebug() << "Updating DataManager & DataStorage";
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(newLSImage);
    // [NOT TRUE ANY MORE] Add and remove need to happen in this order to not get the subject removed
    // if there is only this image in the subject
    long newIid = this->AddMitkNodeToSubject(uid, dataNode, 
        newDataPath, newSpecialRole, "LabelSetImage", newName,
        false, originalIsVisibleInDataView 
    );
    m_DataManager->RemoveData(iid);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return newIid;
}

mitk::Image::Pointer CustomMitkDataStorage::GetImage(long iid)
{
    qDebug() << "CustomMitkDataStorage::GetImage" << iid;

    if (m_DataManager->GetSubjectIdFromDataId(iid) == -1)
    {
        throw std::invalid_argument( 
            std::string("CustomMitkDataStorage::GetImage: No such image, iid=") + 
            std::to_string(iid)
        );
    }

    if (m_DataManager->GetDataType(iid) != "Image")
    {
        throw std::invalid_argument( 
            std::string("CustomMitkDataStorage::GetImage: Not an image, iid=") + 
            std::to_string(iid)
        );
    }

    long uid = m_DataManager->GetSubjectIdFromDataId(iid);

    if (m_CurrentSubjectID == uid)
    {
        qDebug() << "CustomMitkDataStorage::GetImage: Loading from data storage";
        mitk::DataNode::Pointer node = this->GetNamedNode(std::to_string(iid));
        
        mitk::Image::Pointer image; 
        
        if (node && this->Exists(node) && node->GetData()) 
        { 
            qDebug() << "CustomMitkDataStorage::GetImage: Found image in node with name" 
                     << node->GetName().c_str();
            
            try {
                image = dynamic_cast<mitk::Image*>(node->GetData()); 
            } catch (...) {
                qDebug() << "Can't fetch from node";
                throw std::invalid_argument( 
                    std::string("CustomMitkDataStorage::GetImage: Can't fetch from node")
                );
            }
        }
        else {
            qDebug() << "Problem with node";
            throw std::invalid_argument( 
                std::string("CustomMitkDataStorage::GetImage: No such node")
            );
        }

        qDebug() << "Will return image if it exists";
        if (image) {
            return image;
        }
        else {
            qDebug() << "No such image";
            throw std::invalid_argument( 
                std::string("CustomMitkDataStorage::GetImage: No such image")
            );
        }
    }
    else {
        qDebug() << "Attempting to load from file";
        try {
            return mitk::IOUtil::Load<mitk::Image>(
                m_DataManager->GetDataPath(iid).toStdString()
            );
        } catch (...) {
            qDebug() << "CustomMitkDataStorage::GetImage: Loading from file failed";
            throw std::invalid_argument( 
                std::string("CustomMitkDataStorage::GetImage: Loading from file failed")
            );
        }
    }
}

mitk::LabelSetImage::Pointer CustomMitkDataStorage::GetLabelSetImage(long iid)
{
    qDebug() << "CustomMitkDataStorage::GetLabelSetImage" << iid;

    if (m_DataManager->GetSubjectIdFromDataId(iid) == -1)
    {
        throw std::invalid_argument( 
            std::string("CustomMitkDataStorage::GetLabelSetImage: No such image, iid=") + 
            std::to_string(iid)
        );
    }

    if (m_DataManager->GetDataType(iid) != "LabelSetImage")
    {
        throw std::invalid_argument( 
            std::string("CustomMitkDataStorage::GetLabelSetImage: Not a labels image, iid=") + 
            std::to_string(iid)
        );
    }

    long uid = m_DataManager->GetSubjectIdFromDataId(iid);

    if (m_CurrentSubjectID == uid)
    {
        auto node = this->GetNamedNode(std::to_string(iid));
        if (node && node->GetData()) 
        { 
            return dynamic_cast<mitk::LabelSetImage*>(node->GetData()); 
        }
        else {
            throw std::invalid_argument( 
                std::string("CustomMitkDataStorage::GetImage: No such node")
            );
        }
    }
    else {
        qDebug() << "Attempting to load from file";
        try {
            return mitk::IOUtil::Load<mitk::LabelSetImage>(
                m_DataManager->GetDataPath(iid).toStdString()
            );
        } catch (...) {
            qDebug() << "CustomMitkDataStorage::GetLabelSetImage: Loading from file failed";
            throw std::invalid_argument( 
                std::string("CustomMitkDataStorage::GetLabelSetImage: Loading from file failed")
            );
        }
    }
}

void CustomMitkDataStorage::WriteChangesToFileForAllImagesOfCurrentSubject()
{
    if (m_CurrentSubjectID == -1) { return; }

    QRegExp numberRegExp("\\d*");  // a digit (\d), zero or more times (*)

    mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) 
    {
		QString nodeName = QString(it->Value()->GetName().c_str());
				
		// If the node name can be converted to a number 
		// (all of our node are named with their iid)
		if (numberRegExp.exactMatch(nodeName))
		{
			this->WriteChangesToFileIfNecessary(it.Value());
		}
	}
}

long CustomMitkDataStorage::AddMitkNodeToSubject(long uid, mitk::DataNode::Pointer dataNode, 
    QString path, QString specialRole, QString type, QString name,
    bool external, bool visibleInDataView )
{
    auto uids = m_DataManager->GetAllSubjectIds();

	if(std::find(uids.begin(), uids.end(), uid) == uids.end()) {
    	// uid has been removed (dataNode will be deleted automatically (hopefully))
		return -1;
	}

    if (uid == m_CurrentSubjectID)
    {
        dataNode->GetData()->Modified();
        m_NodesWaitMap[name] = dataNode;

        return m_DataManager->AddDataToSubject(
            uid, path, specialRole, type, name, external, visibleInDataView
        );
    }
    else {
        mitk::IOUtil::Save(
		    dataNode->GetData(), 
		    path.toStdString()
	    );
        
        return m_DataManager->AddDataToSubject(uid, 
            path, specialRole, type, name, external, visibleInDataView
        );
    }
}

void CustomMitkDataStorage::SelectedSubjectChangedHandler(long uid)
{
	qDebug() << QString("CustomMitkDataStorage::SelectedSubjectChangedHandler()") << uid;
    m_CurrentSubjectID = uid;

	// Remove the previous ones
	QRegExp numberRegExp("\\d*");  // a digit (\d), zero or more times (*)
	
	mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) 
    {
		QString nodeName = QString(it->Value()->GetName().c_str());
				
		// If the node name can be converted to a number 
		// (all of our node are named with their iid)
		if (numberRegExp.exactMatch(nodeName))
		{
			qDebug() << "Removing node with name: " << it->Value()->GetName().c_str();
			this->WriteChangesToFileIfNecessary(it.Value());
            //emit MitkNodeAboutToBeDeleted(std::stol(it.Value()->GetName().c_str()));
			this->Remove(it.Value());
		}
	}

	if (uid != -1) 
	{ 
		auto iids = m_DataManager->GetAllDataIdsOfSubject(uid);

		for(const long& iid : iids)
		{
			this->AddToDataStorage(iid);
		} 
	}
}

void CustomMitkDataStorage::DataAddedForSelectedSubjectHandler(long iid)
{
    this->AddToDataStorage(iid);
}

void CustomMitkDataStorage::DataRemovedFromSelectedSubjectHandler(long iid)
{
    auto node = this->GetNamedNode(QString::number(iid).toStdString().c_str());
	//this->WriteChangesToFileIfNecessary(node);
    this->Remove(node);
}

void CustomMitkDataStorage::DataRequestedAsSeedsHandler(long iid)
{
    long uid = m_DataManager->GetSubjectIdFromDataId(iid);

    try {
        long newIid = this->ReAddAsLabelSetImage(iid, "Seeds", true);

        for (const long& tIid : m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Seeds"))
        {
            if (tIid != newIid) { m_DataManager->RemoveData(tIid); }
        }
    } catch (...) {
        qDebug() << "CustomMitkDataStorage::DataRequestedAsSeedsHandler: Exception occured";
    }
}

void CustomMitkDataStorage::DataRequestedAsSegmentationHandler(long iid)
{
    try {
        this->ReAddAsLabelSetImage(iid, "Segmentation", true);
    } catch (...) {
        qDebug() << "CustomMitkDataStorage::DataRequestedAsSegmentationHandler: Exception occured";
    }
}

void CustomMitkDataStorage::ExportDataHandler(long iid, QString fileName)
{
    // TODO: Maybe this should handle showing the window
    // This could happen by having the default value "" as fileName
    // and showing the window if that's the case
    
    // Find the node
	mitk::DataNode::Pointer dataNode;
	mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) {
		if (QString::number(iid).toStdString() == it->Value()->GetName())
		{
			dataNode = it->Value();
			break;
		}
	}
    if (!this->Exists(dataNode))
	{
		qDebug() << "Can't find dataNode for exporting image " << iid;
		return;
	}
	
	// Find output directory for exporting
	QString filePathTemp = fileName;
	filePathTemp.replace("\\", "/", Qt::CaseSensitive);
	QStringList filePathSplit = filePathTemp.split("/");
	QString parentDirOfFile = filePathSplit.value(filePathSplit.length() - 2);
	qDebug() << "Parent dir of file" << parentDirOfFile;

	// Create output directory if it doesn't exist
	if (!QDir(parentDirOfFile).exists())
	{
		if (!QDir().mkpath(parentDirOfFile)) { 
			QMessageBox::warning(nullptr, 
				tr("Problem exporting"),
				tr("No permission to write in this directory")
			);
			return; 
		}
	}

    // Save
	qDebug() << "Will save image to:" << fileName;
    try {
        mitk::IOUtil::Save(
		    dataNode->GetData(), 
		    fileName.toStdString()
	    );
    } catch (mitk::Exception& e) {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        QMessageBox::information(nullptr, "Exporting data", 
            "Could not export data. Please check if you have write access to the directory.\n"
        );
    }
}

void CustomMitkDataStorage::AddToDataStorage(long iid)
{
    QString type = m_DataManager->GetDataType(iid);
    if (type != "Image" && type != "LabelSetImage") { return; }
	
	QString specialRole = m_DataManager->GetDataSpecialRole(iid);
	QString dataPath    = m_DataManager->GetDataPath(iid);

    // "Mask" and "Segmentation" should be nrrd
	if ((specialRole == "Mask" || specialRole == "Segmentation") &&
		!dataPath.endsWith(".nrrd", Qt::CaseSensitive)
	) {
        qDebug() << "CustomMitkDataStorage: \"Mask\" or \"Segmentation\" was not nrrd";
		return;
	}

	qDebug() << "CustomMitkDataStorage::AddToDataStorage: Adding iid" << iid;
	QString name = m_DataManager->GetDataName(iid);

    // See if this is the result of waiting for the DataManager to update
    // when the node exists
    mitk::DataNode::Pointer dataNode;
    for (const QString& tName : IdsOfMap<QString, mitk::DataNode::Pointer>(m_NodesWaitMap))
    {
        if (tName == name)
        {
            qDebug() << "Found node in wait map.";
            qDebug() << "wait map previous size: " << m_NodesWaitMap.size();
            dataNode = m_NodesWaitMap[tName];
            m_NodesWaitMap.erase(tName);
            qDebug() << "wait map new size: " << m_NodesWaitMap.size();
            break;
        }
    }

    if (!dataNode)
    {
        dataNode = mitk::DataNode::New();
        qDebug() << "The dataNode doesn't already exist";
        qDebug() << "Data path:" << dataPath;
        if (type == "Image") {
            dataNode->SetData(
                mitk::IOUtil::Load<mitk::Image>(dataPath.toStdString())
            );
        }
        else {
            dataNode->SetData(
                mitk::IOUtil::Load<mitk::LabelSetImage>(dataPath.toStdString())
            );
        }
        qDebug() << "Added image";
    }

	dataNode->SetName(QString::number(iid).toStdString().c_str());
    this->Add(dataNode);
    //dataNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));
	dataNode->SetVisibility(false);

    //dataNode->SetProperty("fixedLayer", mitk::BoolProperty::New(true));
    //dataNode->SetProperty("layer", mitk::IntProperty::New(2));
	
    emit MitkLoadedNewNode(iid, dataNode);
}

void CustomMitkDataStorage::WriteChangesToFileIfNecessary(mitk::DataNode::Pointer dataNode)
{
    if (!dataNode) { return; }
    
    long iid = QString(dataNode->GetName().c_str()).toLong();
    
    if (m_DataManager->GetDataIsExternal(iid)) { return; }
    
    QString type = m_DataManager->GetDataType(iid);
    
    if (type == "Image" || type == "LabelSetImage")
    {
        mitk::IOUtil::Save(
            dataNode->GetData(), 
            m_DataManager->GetDataPath(iid).toStdString()
        );
    }
}

CustomMitkDataStorage::CustomMitkDataStorage() : QObject(nullptr) 
{

}

DataManager* CustomMitkDataStorage::m_DataManager;

long         CustomMitkDataStorage::m_CurrentSubjectID;

std::map<QString, mitk::DataNode::Pointer> CustomMitkDataStorage::m_NodesWaitMap;