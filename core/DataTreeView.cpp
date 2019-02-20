#include "DataTreeView.h"

#include <QDebug>
#include <QGridLayout>
#include <QString>
#include <QMenu>

#include <algorithm>

DataTreeView::DataTreeView(QWidget *parent) : DataViewBase(parent)
{
	// Set the QTreeWidget
	m_TreeWidget = new QTreeWidget(this);
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(m_TreeWidget, 0, 0);
	this->setLayout(layout);
	m_TreeWidget->setStyleSheet("QTreeWidget {background-color:rgb(97,97,97)}"
		"QHeaderView::section {"                          
		    "color: black;"                            
		    "padding: 2px;"                              
		    "height:20px;"                              
		    "border: 0px solid #c30000;"                  
		    "border-left:0px;"                 
		    "border-right:0px;"                           
		    "background: #ff3d00;"
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
	subjectToAdd->setText(0, m_DataManager->GetSubjectName(uid));
	subjectToAdd->setData(0, ID, uid);

	m_Subjects[uid] = subjectToAdd;

	std::vector<long> iids = m_DataManager->GetAllDataIdsOfSubject(uid);

	for (long& iid : iids)
	{
		QTreeWidgetItem* dataToAdd = new QTreeWidgetItem(subjectToAdd);
		//dataToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		dataToAdd->setFlags(Qt::NoItemFlags);
		dataToAdd->setSelected(true);
		dataToAdd->setText(0, m_DataManager->GetDataName(iid));
		dataToAdd->setData(0, ID, iid);
		dataToAdd->setData(0, IS_CHECKED, true);

		m_Data[iid] = dataToAdd;
	}

	if (m_CurrentSubjectID == -1)
	{
		SwitchExpandedView(subjectToAdd);
		m_CurrentSubjectID = uid;

		if (iids.size() > 0) {
			m_CurrentDataID = iids[0];
			qDebug() << "Emit DataTreeView::SelectedDataChanged" << iids[0];
			emit SelectedDataChanged(iids[0]);
		}
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
				dataToAdd->setData(0, ID, iid);
				dataToAdd->setData(0, IS_CHECKED, true);

				if (m_DataManager->GetDataSpecialRole(iid) != QString())
				{
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

void DataTreeView::UpdateProgressHandler(long uid, int progress)
{
	// TODO
}

void DataTreeView::OnItemClick(QTreeWidgetItem *item, int column)
{
	qDebug() << QString("Clicked tree item.");
	bool isTopLevelItem = (!item->parent());

	// If it's a data item and it's check state changed
	if (isTopLevelItem && item->checkState(0) != item->data(0, IS_CHECKED))
	{
		item->setData(0, IS_CHECKED, (item->checkState(0) == Qt::Checked)? true : false);

		qDebug() << "Emit DataTreeView::DataCheckedStateChanged";
		emit DataCheckedStateChanged(item->data(0, ID).toLongLong(), item->data(0, IS_CHECKED).toBool()); 
	}

	QTreeWidgetItem* currentSelected = m_TreeWidget->currentItem();

	QTreeWidgetItem* currentTopLevelItem = (
		(currentSelected->parent()) ?
			currentSelected->parent() :
			currentSelected
	);

	long currentSubjectID = currentTopLevelItem->data(0, ID).toLongLong();

	// If the subject generally changed
	if (m_CurrentSubjectID != currentSubjectID)
	{
		m_CurrentSubjectID = currentSubjectID;
		qDebug() << "Emit DataTreeView::SelectedSubjectChanged" << currentSubjectID;
		emit SelectedSubjectChanged(currentSubjectID);
	}

	// If it's a subject
	if (!currentSelected->parent()) {
		SwitchExpandedView(currentTopLevelItem);
	}

	// If it's a data item and it got checked
	if (currentSelected->parent() && currentSelected->checkState(0) == Qt::Checked)
	{
		qDebug() << "Emit DataTreeView::SelectedDataChanged" << currentSelected->data(0, ID).toLongLong();
		emit SelectedDataChanged(currentSelected->data(0, ID).toLongLong());
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

  // if (ui->patientTree->topLevelItemCount() > 0)
  // {
	 //  SwitchExpandedView(ui->patientTree->topLevelItem(0));
  // }
  //this->ui->stackedWidget->setCurrentIndex(0);
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
	// We are concerned about top level items (subjects)
	if (focusItem && focusItem->parent())
	{
		focusItem = focusItem->parent();
	}

	if (focusItem)
	{
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
}