#include "DataTreeView.h"

#include <QDebug>
#include <QGridLayout>
#include <QString>
#include <QMenu>
#include <QLabel>
#include <QVariant>
#include <QFileDialog>
#include <QMessageBox>
#include <QStackedLayout>
#include <QHeaderView>

#include <algorithm>

DataTreeView::DataTreeView(QWidget *parent) : DataViewBase(parent)
{
	// Set the QTreeWidget
	m_TreeWidget = new QTreeWidget(this);
	m_TreeWidget->header()->setSectionResizeMode(QHeaderView::Fixed);
	GuiModuleBase::PlaceWidgetInWidget(m_TreeWidget, this);

	// TreeWidget columns
	QStringList columnNames = QStringList() << "  Select subjects";
	m_TreeWidget->setHeaderLabels(columnNames);
	m_TreeWidget->setColumnCount(1);

	// Signals and slots
	m_TreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_TreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		this, SLOT(OnItemClick(QTreeWidgetItem*, int))
	);
	connect(m_TreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(OnItemRightClick(const QPoint&))
	);
}

bool DataTreeView::IsDataChecked(long iid)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		return m_Data[iid]->data(0, IS_CHECKED).toBool();
	}

	return false;
}

void DataTreeView::SetDataCheckedState(long iid, bool checkState, bool imitateClick)
{
	qDebug() << "DataTreeView::SetDataCheckedState" << iid;
	if(m_Data.find(iid) == m_Data.end()) { return; }

	auto item = m_Data[iid];
	item->setCheckState(0, (checkState)? Qt::Checked : Qt::Unchecked);

	bool dataCheckStateChanged = (
		!item->checkState(0) != !item->data(0, IS_CHECKED).toBool() // XOR
	);

	if (!dataCheckStateChanged) { return; }

	bool subjectChanged = (
		this->GetDataManager()->GetSubjectIdFromDataId(iid) != m_CurrentSubjectID
	);

	if (subjectChanged && !imitateClick) { return; }

	if (imitateClick)
	{
		this->OnItemClick(item, 0);
	}
	else {
		qDebug() << "emit DataCheckedStateChanged" << iid;
		m_Data[iid]->setData(0, IS_CHECKED, checkState);
		emit DataCheckedStateChanged(iid, checkState);
	}	
}

void DataTreeView::SubjectAddedHandler(long uid)
{
	// Create subject item
	QTreeWidgetItem* subjectToAdd = new QTreeWidgetItem(m_TreeWidget);
	subjectToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	subjectToAdd->setSelected(false);
    subjectToAdd->setData( 0, ID, QVariant(static_cast<long long int>(uid)) );

	// Setup subject name label
	QLabel* subjectNameLabel = new QLabel(m_DataManager->GetSubjectName(uid));
	subjectNameLabel->setObjectName(QString("SubjectName") + QString::number(uid));
	subjectNameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	// subjectNameLabel->setMinimumHeight(35);
	// subjectNameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	// subjectNameLabel->setAlignment(Qt::AlignHCenter);

	// Setup progress bar
	QProgressBar* progressBar = new QProgressBar(this);
    progressBar->setObjectName(QString("ProgressBar") + QString::number(uid));
	progressBar->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
    // progressBar->setMinimumWidth(10);
	// progressBar->setMinimumHeight(10);
	// progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	progressBar->setAlignment(Qt::AlignCenter);

	// Setup progress text
	QLabel* progressText = new QLabel(this);
    progressText->setObjectName(QString("ProgressText") + QString::number(uid));
	progressText->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	// progressText->setMinimumWidth(30);
	// progressText->setMinimumHeight(20);
	// progressText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	progressText->setAlignment(Qt::AlignCenter);

	// Setup progress bar + progress text container
	QWidget* progressBarAndText = new QWidget();
	progressBarAndText->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	// progressBarAndText->setMinimumWidth(10);
	// progressBarAndText->setMinimumHeight(10);
	// progressBarAndText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	progressBarAndText->setFixedSize(/*w*/45, /*h*/20);
	progressBarAndText->setStyleSheet("background-color: none;");
	
	// Add progress bar + progress text to container
	// QStackedLayout* progressLayout = new QStackedLayout();
	QStackedLayout* progressLayout = new QStackedLayout();
	progressLayout->setStackingMode(QStackedLayout::StackingMode::StackAll);
	progressLayout->setContentsMargins(0,0,0,0);
	progressLayout->addWidget(progressText);
	progressLayout->addWidget(progressBar);
	progressBarAndText->setLayout(progressLayout);
	
	// Hide progress information (for now)
	progressText->hide();
	progressBar->hide();

	// Setup a container for everything
	QWidget* allTogether = new QWidget();
	allTogether->setStyleSheet("background-color: none;");
		// allTogether->setAutoFillBackground(true);
	allTogether->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	allTogether->setMinimumHeight(35);
	allTogether->setMinimumWidth(m_TreeWidget->header()->sectionSize(0) - 20);
	allTogether->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	// Setup a layout for everything
	QHBoxLayout* hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(7,9,7,9);
	hLayout->setSpacing(15);
	hLayout->addWidget(subjectNameLabel, Qt::AlignLeft);
	hLayout->addWidget(progressBarAndText, Qt::AlignRight);
	allTogether->setLayout(hLayout);

	// Add container to widget
	m_TreeWidget->setItemWidget(subjectToAdd, 0, allTogether);

	// For text overflow in the subject name
	// https://stackoverflow.com/questions/7381100/text-overflow-for-a-qlabel-s-text-rendering-in-qt
	// (Doesn't work)
	// QFontMetrics metrics(subjectNameLabel->font());
	// int eWidth = subjectNameLabel->width() - 2;
	// QString clippedText = metrics.elidedText(subjectNameLabel->text(), Qt::ElideRight, eWidth);
	// qDebug() << "Clipped text" << clippedText;
	// subjectNameLabel->setText(clippedText);

	// Add to subjects map
	m_Subjects[uid] = subjectToAdd;

	// See whether to emit SelectedSubjectChanged

	auto allUids = m_DataManager->GetAllSubjectIds();
	bool currentSubjectIdExists = false;
	for (const auto& oneUid : allUids)
	{
		if (oneUid == m_CurrentSubjectID) {
			currentSubjectIdExists = true;
			break;
		}
	}

	if (!currentSubjectIdExists)
	{
		m_TreeWidget->setCurrentItem(subjectToAdd);
		subjectToAdd->setSelected(true);
		SwitchExpandedView(subjectToAdd);
		m_CurrentSubjectID = uid;
		qDebug() << "Emit DataTreeView::SelectedSubjectChanged";
		emit SelectedSubjectChanged(uid);
	}
}

void DataTreeView::SubjectRemovedHandler(long uid)
{
	qDebug() << "DataTreeView::SubjectRemovedHandler" << QString::number(uid);
	QTreeWidgetItem* subjectToRemove = m_Subjects[uid];
	m_CurrentDataID = -1;

	qDebug() << "Emit DataTreeView::SelectedDataChanged" << -1;
	emit SelectedDataChanged(-1);

	if (subjectToRemove)
	{
		//auto iter = m_Subjects.find(uid);
  	m_Subjects.erase(uid);
		delete subjectToRemove;
	}

	if (m_CurrentSubjectID == uid)
	{
		qDebug() << "m_Subjects size" << m_Subjects.size();
		qDebug() << "m_DataManager->GetAllSubjectIds() size" << m_DataManager->GetAllSubjectIds().size();
		if (m_Subjects.size() > 0)
		{
			qDebug() << "New selected subject will be " << m_DataManager->GetAllSubjectIds()[0]; 
			QTreeWidgetItem* newSelected = m_Subjects[m_DataManager->GetAllSubjectIds()[0]];
		
			m_TreeWidget->setCurrentItem(newSelected);
			SwitchExpandedView(newSelected);
			m_CurrentSubjectID = m_DataManager->GetAllSubjectIds()[0];
		}
		else {
			qDebug() << "No new selected subject";
			m_CurrentSubjectID = -1;
		}

		qDebug() << "Emit DataTreeView::SelectedSubjectChanged" << m_CurrentSubjectID;
		emit SelectedSubjectChanged(m_CurrentSubjectID);
	}
}

void DataTreeView::SubjectDataChangedHandler(long uid)
{
	QTreeWidgetItem* subject = m_Subjects[uid];

	if (!subject) { return; }

	std::vector<long> iids = m_DataManager->GetAllDataIdsOfSubject(uid);

	// Check if any new data were loaded
	for (const long& iid : iids)
	{
		if (m_Data.find(iid) != m_Data.end())
		{
			// Data is already in tree view
			continue;
		}

		QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
		QString name        = this->GetDataManager()->GetDataName(iid);
		QString filePath    = this->GetDataManager()->GetDataPath(iid);

		// Add new data in tree view
		QTreeWidgetItem* dataToAdd = new QTreeWidgetItem(subject);
		dataToAdd->setCheckState(0, Qt::Unchecked);
		dataToAdd->setData(0, ID, QVariant(static_cast<long long int>(iid)));
		dataToAdd->setData(0, IS_CHECKED, false);

		if (specialRole != "" && name == "")
		{
			// Data has a special role
			dataToAdd->setText(0, 
				QString("<") + specialRole + QString(">")
			);
		}
		else {
			// No special role, set the name of the image
			dataToAdd->setText(0, m_DataManager->GetDataName(iid));
		}

		m_Data[iid] = dataToAdd;

		// Broadcast what happened if necessary
		if (m_CurrentSubjectID == uid)
		{
			qDebug() << "Emit DataTreeView::DataAddedForSelectedSubject" << iid;
			emit DataAddedForSelectedSubject(iid);
		
			if (specialRole == "Seeds" || specialRole == "Segmentation") 
			{
				dataToAdd->setCheckState(0, Qt::Checked);
				dataToAdd->setData(0, IS_CHECKED, true);

				emit DataCheckedStateChanged(iid, true);

				dataToAdd->setSelected(true);
				m_CurrentDataID = iid;
				m_TreeWidget->setCurrentItem(dataToAdd);
				emit SelectedDataChanged(iid);
			}

			if (specialRole == "Segmentation")
			{
				for (const long& dIid : m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
					uid, "Seeds"))
				{
					if (m_Data[dIid] && m_Data[dIid]->data(0, IS_CHECKED) == true) 
					{
						m_Data[dIid]->setCheckState(0, Qt::Unchecked);
						m_Data[dIid]->setData(0, IS_CHECKED, false);
						emit DataCheckedStateChanged(dIid, false);
					}
				}

				for (const long& dIid : m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
					uid, "Segmentation"))
				{
					if (dIid == iid) { continue; }

					if (m_Data[dIid] && m_Data[dIid]->data(0, IS_CHECKED) == true) {
						m_Data[dIid]->setCheckState(0, Qt::Unchecked);
						m_Data[dIid]->setData(0, IS_CHECKED, false);
						emit DataCheckedStateChanged(dIid, false);
					}
				}
			}
		}
	}
	
	// Check if any data were removed
	for (int i = 0; i < subject->childCount(); i++)
	{
		long iid = subject->child(i)->data(0, ID).toLongLong();
		
		if (std::find(iids.begin(), iids.end(), iid) == iids.end())
		{
			delete m_Data[iid];

			if (m_CurrentSubjectID == uid)
			{
				bool wasTheSelectedData = (m_CurrentDataID == iid);
				if (m_TreeWidget->currentItem() && m_TreeWidget->currentItem()->parent())
				{
					m_CurrentDataID = m_TreeWidget->currentItem()->data(0, ID).toLongLong();
				}
				else {
					m_CurrentDataID = -1;
				}

				if (wasTheSelectedData) { 
					qDebug() << "Emit DataTreeView::SelectedDataChanged" << -1;
					emit SelectedDataChanged(-1); 
				}

				qDebug() << "Emit DataTreeView::DataRemovedFromSelectedSubject" << iid;
				emit DataRemovedFromSelectedSubject(iid);
			}
		}
	}

	// Last resort check
	QTreeWidgetItem* current = m_TreeWidget->currentItem();
	if (!current || !current->parent())
	{
		m_CurrentDataID = -1;
	}
	else {
		m_CurrentDataID = current->data(0, ID).toLongLong();
	}
}

void DataTreeView::UpdateProgressHandler(long uid, QString message, int progress)
{

	auto uids = this->GetDataManager()->GetAllSubjectIds();

	if(std::find(uids.begin(), uids.end(), uid) == uids.end()) {
    	// uid has been removed
		return;
	}

  	QProgressBar *progressBar = m_TreeWidget->findChild<QProgressBar*>(
		QString("ProgressBar") + QString::number(uid)
	);

	QLabel *progressText = m_TreeWidget->findChild<QLabel*>(
		QString("ProgressText") + QString::number(uid)
	);

	if (message == "Queued")
	{
		qDebug() << "DataTreeView::UpdateProgressHandler" << "Queued";

		progressBar->setValue(0);	
		progressBar->hide();
		
		progressText->setText("<i>" + message + "</i>");
		progressText->show();
	
		return;
	}
	
	progressText->setText("");
	progressText->hide();
	
	if (progress == -1)
	{
		progressBar->setValue(0);
		progressBar->hide();
	}
	else {
		progressBar->setValue(progress);
		progressBar->show();
	}
}

void DataTreeView::OnItemClick(QTreeWidgetItem *item, int column)
{
	qDebug() << QString("Clicked tree item.");

	// if (!m_TreeWidget->currentItem())
	// {
	// 	item->setSelected(true);
	// 	m_TreeWidget->setCurrentItem(item);
	// }

	bool isTopLevelItem = (!item->parent());

	if (!isTopLevelItem)
	{
		// If it's a data item

		if(item->checkState(0) == Qt::Checked)
		{
			item->setSelected(true);
			m_TreeWidget->setCurrentItem(item);
		}
		
		
		long iid = item->data(0, ID).toLongLong();
		long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);

		bool dataChanged = (iid != m_CurrentDataID)/* && (item->checkState(0) == Qt::Checked)*/;
		bool subjectChanged = (uid != m_CurrentSubjectID);
		bool dataCheckStateChanged = (
			!item->checkState(0) != !item->data(0, IS_CHECKED).toBool() // XOR
		);

		m_CurrentDataID = iid;

		// If the active current data item actually changed and it's active
		if (subjectChanged && dataChanged)
		{
			// Just to be sure
			qDebug() << "Emit DataTreeView::SelectedDataChanged" << -1;
			emit SelectedDataChanged(-1);
						
			// Switch to this subject and then this data
			SwitchExpandedView(item->parent());
			m_CurrentSubjectID = uid;
			if (dataCheckStateChanged) { // The item was enabled, but SwitchExpandedView disabled it
				item->setData(0, IS_CHECKED, true);
				item->setCheckState(0, Qt::Checked);
			}
			item->parent()->setSelected(false);
			item->setSelected(true);
			m_TreeWidget->setCurrentItem(item);
			qDebug() << "Emit DataTreeView::SelectedSubjectChanged";
			emit SelectedSubjectChanged(uid);
		}

		qDebug() << "Emit DataTreeView::SelectedDataChanged" << iid;
		emit SelectedDataChanged(iid);

		if (dataCheckStateChanged)
		{
			bool newCheckState = (item->checkState(0)) ? true : false;
			item->setData(0, IS_CHECKED, newCheckState);

			qDebug() << "Emit DataTreeView::DataCheckedStateChanged";
			emit DataCheckedStateChanged(iid, newCheckState);			
		}
	}
	else {
		// If it's a subject item

		long uid = item->data(0, ID).toLongLong();


		if (uid != m_CurrentSubjectID)
		{
			m_CurrentSubjectID = uid;

			qDebug() << "Emit DataTreeView::SelectedSubjectChanged";
			emit SelectedSubjectChanged(uid);
			SwitchExpandedView(item);			
		}

		m_CurrentDataID = -1;
		qDebug() << "Emit DataTreeView::SelectedDataChanged" << m_CurrentDataID;
		emit SelectedDataChanged(m_CurrentDataID);
	}
}

void DataTreeView::OnItemRightClick(const QPoint& pos)
{
	qDebug() << QString("Show context menu pressed");

	if (!m_TreeWidget->itemAt(pos))
	{
		return;
	}

	QMenu contextMenu(m_TreeWidget);
	QPoint posForContextMenu = QPoint(pos);

	QAction action1("Remove", &contextMenu);
	connect(&action1, SIGNAL(triggered()), this, SLOT(OnItemRightClickRemove()));
	contextMenu.addAction(&action1);

	if (m_TreeWidget->itemAt(pos)->parent()) 
	{
		// The item is an image

		long iid = m_TreeWidget->itemAt(pos)->data(0, ID).toLongLong();
		qDebug() << "iid right clicked:" << iid;

		// Check if the right click didn't occur of the current subject
		if (this->GetDataManager()->GetSubjectIdFromDataId(iid) != m_CurrentSubjectID)
		{
			qDebug() << "Right click has to change subject";

			auto item = m_TreeWidget->itemAt(pos);
			this->OnItemClick(item, 0);

			// Find the new position of the item
			QRect itemRect = m_TreeWidget->visualItemRect(item);
			posForContextMenu = QPoint(itemRect.x(), itemRect.y());
		}

		QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
		if (specialRole != "Seeds")
		{
			QAction* action2 = new QAction("Set as seeds", &contextMenu);
			connect(action2, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsSeeds()));
			contextMenu.addAction(action2);
		}
		
		if (specialRole != "Seeds" && specialRole != "Segmentation")
		{
			QAction* action22 = new QAction("Convert to segmentation", &contextMenu);
			connect(action22, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsSegmentation()));
			contextMenu.addAction(action22);
		}
		else if (specialRole == "Segmentation" && 
			this->GetDataManager()->GetAllDataIdsOfSubjectWithSpecialRole(
				m_CurrentSubjectID, "Seeds").size() > 0)
		{
			QAction* action22 = new QAction("Sync colors from seeds", &contextMenu);
			connect(action22, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsSegmentation()));
			contextMenu.addAction(action22);
		}
		
		if (specialRole == "Seeds" || specialRole == "Segmentation")
		{
			QAction* action3 = new QAction("Export", &contextMenu);
			connect(action3, SIGNAL(triggered()), this, SLOT(OnItemRightClickExport()));
			contextMenu.addAction(action3);
		}
	}

	contextMenu.exec(m_TreeWidget->viewport()->mapToGlobal(pos));
}

void DataTreeView::OnItemRightClickRemove()
{
  qDebug() << QString("Trying to delete item");

  if (m_TreeWidget->currentItem()->parent())
  {
  	m_DataManager->RemoveData(m_TreeWidget->currentItem()->data(0, ID).toLongLong());
  }
  else {
  	m_DataManager->RemoveSubject(m_TreeWidget->currentItem()->data(0, ID).toLongLong());
  }
}

void DataTreeView::OnItemRightClickSetAsSeeds()
{
	long iid = m_TreeWidget->currentItem()->data(0, ID).toLongLong();
	emit DataRequestedAsSeeds(iid);
}

void DataTreeView::OnItemRightClickSetAsSegmentation()
{
	long iid = m_TreeWidget->currentItem()->data(0, ID).toLongLong();
	emit DataRequestedAsSegmentation(iid);
}

void DataTreeView::OnItemRightClickExport()
{
	qDebug("DataTreeView::OnItemRightClickExport");
	long iid = m_TreeWidget->currentItem()->data(0, ID).toLongLong();
	QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
	long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);
	QString originalSubjectPath = this->GetDataManager()->GetOriginalSubjectPath(uid);
	
	QFileInfo f(this->GetDataManager()->GetDataPath(iid));
	QString baseName = f.baseName();

	QString fileTypes;
	for(int i = 0; i < m_AcceptedFileTypes.size(); i++)
	{
		fileTypes += m_AcceptedFileTypes.at(i).mid(2); // Without the first two character
		fileTypes += "(" + m_AcceptedFileTypes.at(i) + ")";

		if (i < m_AcceptedFileTypes.size() - 1) { fileTypes += ";;"; }
	}

	qDebug() << "Filetypes: " << fileTypes;

	if (!this->GetDataManager()->GetDataIsExternal(iid) && 
	    this->GetDataManager()->GetDataSpecialRole(iid) != "")
	{
		baseName = this->GetDataManager()->GetDataSpecialRole(iid).toLower();
	}

	// Show Dialog to get desired name
	QString fileName = QFileDialog::getSaveFileName(this, 
		QString("Save ") + specialRole.toLower(),
		originalSubjectPath.replace("\\", "/", Qt::CaseSensitive) + QString("/") + baseName + QString(".nii.gz"), 
		fileTypes // tr("Images (*.nii.gz)")
	);
	
	if (fileName.isEmpty())
	{
		// Basically canceled
		return;
	}

	qDebug() << "Filename for export (before potentially adding .nii.gz): " << fileName;
  
	// Check if the filename ends with a supported file type.
	// If it doesn't add default.
	// This is because QFileDialog::getSaveFileName allows this on linux.
	// On Windows it forces one of the filetypes in filter.
	bool foundFileType = false;
	foreach(QString ft, m_AcceptedFileTypes)
	{
		if (fileName.endsWith(ft.mid(1))) { 
			foundFileType = true; 
			break; 
		}
	}
	if (!foundFileType)
	{
		fileName = fileName + ".nii.gz";
	}

	emit ExportData(iid, fileName);
}

void DataTreeView::SwitchExpandedView(QTreeWidgetItem* focusItem)
{
	qDebug() << "DataTreeView::SwitchExpandedView()";

	if (!focusItem) { return; }
	
	// We are concerned about top level items (subjects)
	if (focusItem->parent())
	{
		focusItem = focusItem->parent();
	}

	// Uncheck everything from last focused subject
	if (m_CurrentSubjectID != -1 && m_Subjects.find(m_CurrentSubjectID) != m_Subjects.end())
	{
		for (int i = 0; i < m_Subjects[m_CurrentSubjectID]->childCount(); i++)
		{
			qDebug() << "Setting things to false";
			m_Subjects[m_CurrentSubjectID]->child(i)->setCheckState(0, Qt::Unchecked);
			m_Subjects[m_CurrentSubjectID]->child(i)->setData(0, IS_CHECKED, false);
		}
	}

	// Uncheck everything from current focused subject
	for (int i = 0; i < focusItem->childCount(); i++)
	{
		qDebug() << "Setting stuff to false";
		focusItem->child(i)->setCheckState(0, Qt::Unchecked);
		focusItem->child(i)->setData(0, IS_CHECKED, false);
	}

	focusItem->setSelected(true);
	focusItem->setExpanded(true);
	int focusItemIndex = m_TreeWidget->indexOfTopLevelItem(focusItem);

	for (int i = 0; i < m_TreeWidget->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* topLevelItem = m_TreeWidget->topLevelItem(i);
			
		if (i != focusItemIndex)
		{
			qDebug() << "Un-expanding item";
			m_TreeWidget->topLevelItem(i)->setSelected(false);
			m_TreeWidget->topLevelItem(i)->setExpanded(false);
		}
	}
}
