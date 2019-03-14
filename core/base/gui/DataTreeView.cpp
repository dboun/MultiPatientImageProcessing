#include "DataTreeView.h"

#include <QDebug>
#include <QGridLayout>
#include <QString>
#include <QMenu>
#include <QLabel>
#include <QVariant>

#include <algorithm>

DataTreeView::DataTreeView(QWidget *parent) : DataViewBase(parent)
{
	// Set the QTreeWidget
	m_TreeWidget = new QTreeWidget(this);
	GuiModuleBase::PlaceWidgetInWidget(m_TreeWidget, this);
	
	m_TreeWidget->setStyleSheet("QTreeWidget {background-color:rgb(97,97,97);border:1px solid rgb(255,61,0);border-radius:2px}"
		"QHeaderView::section {"                          
		    "color: black;"                            
		    "background-color:#000000;"
		    "padding: 4px;"                              
		    "height:20px;"                              
		    "border: 0px solid #000000;"                  
		    "border-bottom:1px;"                                            
		    "background: #9f0000;"
		"}"
	);

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

void DataTreeView::SubjectAddedHandler(long uid)
{
	QTreeWidgetItem* subjectToAdd = new QTreeWidgetItem(m_TreeWidget);
	subjectToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	subjectToAdd->setSelected(true);
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
	progressBar->setStyleSheet("QProgressBar {"
		"background-color: #4f4f4f;"
		"color: #9a9a9a;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-color: #323232;"
		"border-radius: 4px;"
		"text-align: center; }"

		"QProgressBar::chunk {"
		"background-color: #F5F5F5; }"
	);

	QLabel *label = new QLabel(m_DataManager->GetSubjectName(uid));
	label->setMaximumHeight(20);
	label->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	QWidget *labelAndProgress = new QWidget();
	labelAndProgress->setStyleSheet("background-color: rgba(0,0,0,0)");
	labelAndProgress->setAutoFillBackground(true);
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
		SwitchExpandedView(subjectToAdd);
		m_CurrentSubjectID = uid;
		qDebug() << "Emit DataTreeView::SelectedSubjectChanged";
		emit SelectedSubjectChanged(uid);
	}
}

void DataTreeView::SubjectRemovedHandler(long uid)
{
	QTreeWidgetItem* subjectToRemove = m_Subjects[uid];

	if (subjectToRemove)
	{
		m_Subjects.erase(uid);
		delete subjectToRemove;
	}

	if (m_CurrentSubjectID == uid)
	{
		if (m_Subjects.size() > 0)
		{
			QTreeWidgetItem* newSelected = m_Subjects[m_DataManager->GetAllSubjectIds()[0]];
		
			m_TreeWidget->setCurrentItem(newSelected);
			SwitchExpandedView(newSelected);
			m_CurrentSubjectID = m_DataManager->GetAllSubjectIds()[0];
		}
		else {
			m_CurrentSubjectID = -1;
		}

		qDebug() << "Emit DataTreeView::SelectedSubjectChanged";
		emit SelectedSubjectChanged(m_CurrentSubjectID);
	}
}

void DataTreeView::SubjectDataChangedHandler(long uid)
{
	QTreeWidgetItem* subject = m_Subjects[uid];

	if (subject)
	{
		std::vector<long> iids = m_DataManager->GetAllDataIdsOfSubject(uid);

		for (long& iid : iids)
		{
			if (m_Data.find(iid) == m_Data.end())
			{
				QTreeWidgetItem* dataToAdd = new QTreeWidgetItem(subject);
				//dataToAdd->setSelected(true);
				dataToAdd->setCheckState(0, Qt::Unchecked);
				dataToAdd->setText(0, m_DataManager->GetDataName(iid));
                dataToAdd->setData(0, ID, QVariant(static_cast<long long int>(iid)));
				dataToAdd->setData(0, IS_CHECKED, false);

				QString specialRole = m_DataManager->GetDataSpecialRole(iid);
				if (specialRole != QString())
				{
					if (specialRole == "Mask")
					{
						for (int i = 0; i < subject->childCount(); i++)
						{
							QTreeWidgetItem* item = subject->child(i);
							long itemId = item->data(0, ID).toLongLong();
							QString itemSpecialRole = m_DataManager->GetDataSpecialRole(itemId);
							
							if (itemSpecialRole == "Mask")
							{
								item->setText(0, m_DataManager->GetDataName(itemId));
							}
						}
					}

					if (specialRole == "Segmentation")
					{
						for (int i = 0; i < subject->childCount(); i++)
						{
							QTreeWidgetItem* item = subject->child(i);
							long itemId = item->data(0, ID).toLongLong();
							QString itemSpecialRole = m_DataManager->GetDataSpecialRole(itemId);

							if (itemSpecialRole == "Segmentation")
							{
								m_DataManager->RemoveData(itemId);
							}
						}
					}

					dataToAdd->setText(0, "<" + m_DataManager->GetDataSpecialRole(iid) + ">");
				}

				m_Data[iid] = dataToAdd;	

				if (m_CurrentSubjectID == uid)
				{
					qDebug() << "Emit DataTreeView::DataAddedForSelectedSubject" << iid;
					emit DataAddedForSelectedSubject(iid);
				}
			}
		}
		
		for (int i = 0; i < subject->childCount(); i++)
		{
			long iid = subject->child(i)->data(0, ID).toLongLong();
			if (std::find(iids.begin(), iids.end(), iid) == iids.end())
			{
				delete m_Data[iid];

				if (m_CurrentSubjectID == uid)
				{
					qDebug() << "Emit DataTreeView::DataRemovedFromSelectedSubject" << iid;
					emit DataRemovedFromSelectedSubject(iid);
				}
			}
		}
	}
}

void DataTreeView::UpdateProgressHandler(long uid, QString message, int progress)
{
    QProgressBar *progressBar = m_TreeWidget->findChild<QProgressBar*>(QString("ProgressBar") + QString::number(uid));
	progressBar->setVisible(true);
	progressBar->setValue(progress);
}

void DataTreeView::OnItemClick(QTreeWidgetItem *item, int column)
{
	qDebug() << QString("Clicked tree item.");

	if (!m_TreeWidget->currentItem())
	{
		m_TreeWidget->setCurrentItem(item);
	}

	bool isTopLevelItem = (!item->parent());

	// If it's a data item and it's check state changed
	if (!isTopLevelItem && item->checkState(0) != item->data(0, IS_CHECKED).toBool())
	{
		item->setData(0, IS_CHECKED, (item->checkState(0)) ? true : false);

		qDebug() << "Emit DataTreeView::DataCheckedStateChanged";
		long iid = item->data(0, ID).toLongLong();
		qDebug() << "Emit DataTreeView::SelectedSubjectChanged";
		//emit SelectedSubjectChanged(m_DataManager->GetSubjectIdFromDataId(iid));
		emit DataCheckedStateChanged(iid, item->data(0, IS_CHECKED).toBool()); 
	}

	QTreeWidgetItem* topLevelItem = (
		(item->parent()) ?
			item->parent() :
			item
	);

	// If it's a data item and it got selected and it's checked
	if (item->parent() && m_CurrentDataID != item->data(0, ID).toLongLong()
		&& item->checkState(0) == Qt::Checked)
	{
		m_CurrentDataID = item->data(0, ID).toLongLong();
		qDebug() << "Emit DataTreeView::SelectedDataChanged" << item->data(0, ID).toLongLong();
		emit SelectedDataChanged(item->data(0, ID).toLongLong());
	}

	long currentSubjectID = topLevelItem->data(0, ID).toLongLong();

	// If the subject generally changed
	if (m_CurrentSubjectID != currentSubjectID)
	{
		m_CurrentSubjectID = currentSubjectID;
		qDebug() << "Emit DataTreeView::SelectedSubjectChanged" << currentSubjectID;
		emit SelectedSubjectChanged(currentSubjectID);
	}

	// If it's a subject
	if (isTopLevelItem) {
		SwitchExpandedView(topLevelItem);
	}
}

void DataTreeView::OnItemRightClick(const QPoint& pos)
{
	qDebug() << QString("Show context menu pressed");

	if (m_TreeWidget->itemAt(pos))
	{
		QMenu *contextMenu = new QMenu(m_TreeWidget);

		QAction action1("Remove", this);
		//action1.setShortcutContext(Qt::ApplicationShortcut);
		//action1.setShortcut(QKeySequence::Delete);
		connect(&action1, SIGNAL(triggered()), this, SLOT(OnItemRightClickRemove()));
		contextMenu->addAction(&action1);

		QAction action2("Set as mask", this);
		connect(&action2, SIGNAL(triggered()), this, SLOT(OnItemRightClickSetAsMask()));

		if (m_TreeWidget->itemAt(pos)->parent()) 
		{
		  // The item is an image
		  contextMenu->addAction(&action2);
		}

		contextMenu->exec(m_TreeWidget->viewport()->mapToGlobal(pos));
	}
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

	QString currentRole = m_DataManager->GetDataSpecialRole(iid);

	if (currentRole != QString("Mask"))
	{
		QString name = m_DataManager->GetDataName(iid);
		QString path = m_DataManager->GetDataPath(iid);
		QString type = m_DataManager->GetDataType(iid);
		long uid = m_DataManager->GetSubjectIdFromDataId(iid);

		m_DataManager->RemoveData(iid);
		m_DataManager->AddDataToSubject(uid, path, "Mask", type, name);
	}
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
	if (m_CurrentSubjectID != -1 && m_Subjects[m_CurrentSubjectID])
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
