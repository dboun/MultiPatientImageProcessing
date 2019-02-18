#include "DataTreeView.h"

#include <QDebug>
#include <algorithm>

DataTreeView::DataTreeView(QWidget *parent) : DataViewBase(parent)
{
	m_TreeWidget = new QTreeWidget(this);
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
		dataToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
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
				dataToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				dataToAdd->setSelected(true);
				dataToAdd->setText(0, m_DataManager->GetDataName(iid));
				dataToAdd->setData(0, ID, iid);
				dataToAdd->setData(0, IS_CHECKED, true);

				m_Data[iid] = dataToAdd;				
			}
		}
		
		for (int i = 0; i < subject->childCount(); i++)
		{
			long iid = subject->child(i)->data(0, ID).toLongLong();
			if (std::find(iids.begin(), iids.end(), iid) != iids.end())
			{
				delete m_Data[iid];
			}
		}
	}
}

void DataTreeView::OnItemClick(QTreeWidgetItem *item, int column)
{
	qDebug() << QString("Clicked tree item.");
	bool isTopLevelItem = (!item->parent());

	// If it's a data item and it's check state changed
	if (isTopLevelItem && item->checkState(0) != item->data(0, IS_CHECKED))
	{
		item->setData(0, IS_CHECKED, (item->checkState(0) == Qt::Checked)? true : false);

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
		emit SelectedSubjectChanged(currentSubjectID);
	}

	// If it's a subject
	if (!currentSelected->parent()) {
		SwitchExpandedView(currentTopLevelItem);
	}

	// If it's a data item and it got checked
	if (currentSelected->parent() && currentSelected->checkState(0) == Qt::Checked)
	{
		emit SelectedDataChanged(currentSelected->data(0, ID).toLongLong());
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