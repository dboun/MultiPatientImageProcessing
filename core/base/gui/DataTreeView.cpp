#include "DataTreeView.h"

#include <QDebug>
#include <QGridLayout>
#include <QString>
#include <QMenu>
#include <QLabel>
#include <QVariant>
#include <QFileDialog>
#include <QMessageBox>

#include <algorithm>

DataTreeView::DataTreeView(QWidget *parent) : DataViewBase(parent)
{
	// Set the QTreeWidget
	m_TreeWidget = new QTreeWidget(this);
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

void DataTreeView::SetDataCheckedState(long iid, bool checkState)
{
	if (m_Data.find(iid) != m_Data.end())
	{
		m_Data[iid]->setData(0, IS_CHECKED, checkState);
		m_Data[iid]->setCheckState(0, (checkState)? Qt::Checked : Qt::Unchecked);
	}
}

void DataTreeView::SubjectAddedHandler(long uid)
{
	QTreeWidgetItem* subjectToAdd = new QTreeWidgetItem(m_TreeWidget);
	subjectToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	subjectToAdd->setSelected(false);
	//subjectToAdd->setText(0, m_DataManager->GetSubjectName(uid));
    subjectToAdd->setData(0, ID, QVariant(static_cast<long long int>(uid) ));

	QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setObjectName(QString("ProgressBar") + QString::number(uid));
	progressBar->setVisible(false);
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
    progressBar->setAlignment(Qt::AlignCenter);
	progressBar->setMinimumWidth(30);
	progressBar->setMaximumWidth(40);
	progressBar->setMaximumHeight(20);

	QLabel *label = new QLabel(m_DataManager->GetSubjectName(uid));
	label->setMaximumHeight(20);
	label->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	QWidget *labelAndProgress = new QWidget();
	//labelAndProgress->setAutoFillBackground(true);
	labelAndProgress->setStyleSheet("background-color: none;");
	labelAndProgress->setMaximumHeight(35);
	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addWidget(label, Qt::AlignLeft);
	hLayout->addWidget(progressBar, Qt::AlignRight);
	labelAndProgress->setLayout(hLayout);
	labelAndProgress->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	m_TreeWidget->setItemWidget(subjectToAdd, 0, labelAndProgress);

	m_Subjects[uid] = subjectToAdd;

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
		
			if (specialRole == "Mask" || specialRole == "Segmentation") 
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
					uid, "Mask"))
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
				if (m_TreeWidget->currentItem() && m_TreeWidget->currentItem()->parent())
				{
					m_CurrentDataID = m_TreeWidget->currentItem()->data(0, ID).toLongLong();
				}
				else {
					m_CurrentDataID = -1;
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

  QProgressBar *progressBar = m_TreeWidget->findChild<QProgressBar*>(
		QString("ProgressBar") + QString::number(uid)
	);
	if (progress == -1)
	{
		progressBar->setValue(0);
		progressBar->setVisible(false);
	}
	else {
		progressBar->setVisible(true);
		progressBar->setValue(progress);
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

	QAction action1("Remove", &contextMenu);
	connect(&action1, SIGNAL(triggered()), this, SLOT(OnItemRightClickRemove()));
	contextMenu.addAction(&action1);

	if (m_TreeWidget->itemAt(pos)->parent()) 
	{
		// The item is an image
		long iid = m_TreeWidget->itemAt(pos)->data(0, ID).toLongLong();
		qDebug() << "iid right clicked:" << iid;

		QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
		if (specialRole != "Mask")
		{
			QAction* action2 = new QAction("Set as mask", &contextMenu);
			connect(action2, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsMask()));
			contextMenu.addAction(action2);
		}
		
		if (specialRole != "Mask" && specialRole != "Segmentation")
		{
			QAction* action22 = new QAction("Convert to segmentation", &contextMenu);
			connect(action22, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsSegmentation()));
			contextMenu.addAction(action22);
		}
		else if (specialRole == "Segmentation" && 
			this->GetDataManager()->GetAllDataIdsOfSubjectWithSpecialRole(
				m_CurrentSubjectID, "Mask").size() > 0)
		{
			QAction* action22 = new QAction("Sync colors from mask", &contextMenu);
			connect(action22, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsSegmentation()));
			contextMenu.addAction(action22);
		}
		
		if (specialRole == "Mask" || specialRole == "Segmentation")
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

void DataTreeView::OnItemRightClickSetAsMask()
{
	long iid = m_TreeWidget->currentItem()->data(0, ID).toLongLong();
	emit DataRequestedAsMask(iid);

	// QString currentRole = m_DataManager->GetDataSpecialRole(iid);

	// if (currentRole != QString("Mask"))
	// {
	// 	QString name = m_DataManager->GetDataName(iid);
	// 	QString path = m_DataManager->GetDataPath(iid);
	// 	QString type = m_DataManager->GetDataType(iid);
	// 	long uid = m_DataManager->GetSubjectIdFromDataId(iid);

	// 	m_DataManager->RemoveData(iid);
	// 	m_DataManager->AddDataToSubject(uid, path, "Mask", type, name);
	// }
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

	// Show Dialog to get desired name
	QString fileName = QFileDialog::getSaveFileName(this, 
		QString("Save ") + specialRole.toLower(),
		originalSubjectPath.replace("\\", "/", Qt::CaseSensitive) + QString("/") + baseName + QString(".nii.gz"), 
		tr("Images (*.nii.gz)")
	);
	
	if (fileName.isEmpty())
	{
		// Basically canceled
		return;
	}
  
  if (!fileName.endsWith(".nii.gz"))
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
