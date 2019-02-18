#ifndef DATA_TREE_VIEW_H
#define DATA_TREE_VIEW_H

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <map>

#include "DataViewBase.h"

class DataTreeView : public DataViewBase
{
	Q_OBJECT

public:
	const int ID         = Qt::UserRole;
	const int IS_CHECKED = Qt::UserRole + 1;

	DataTreeView(QWidget *parent = nullptr);

	void SubjectAddedHandler(long uid) override;
	void SubjectRemovedHandler(long uid) override;
	void SubjectDataChangedHandler(long uid) override;

public slots:
	void OnItemClick(QTreeWidgetItem *item, int column);

private:
	void SwitchExpandedView(QTreeWidgetItem* focusItem);

	QTreeWidget* m_TreeWidget;
	std::map<long, QTreeWidgetItem*> m_Subjects;
	std::map<long, QTreeWidgetItem*> m_Data;

	long m_CurrentSubjectID = -1;
	long m_CurrentDataID    = -1;
};

#endif // ! DATA_TREE_VIEW_H
