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
	void UpdateProgressHandler(long uid, int progress) override;

// void MainWindow::HandleConfigButton()
// {
//   // TODO (Not implemented yet)
//   QStringList apps;

// //#ifdef BUILD_GEODESIC_TRAINING
//   //apps << "Geodesic Training Segmentation";
// //#endif // BUILD_GEODESIC_TRAINING

// }

// void MainWindow::OnTreeWidgetClicked(QTreeWidgetItem *item, int column)
// {
//   qDebug() << QString("Clicked tree item.");
  
//   // Make only one expanded
//   QTreeWidgetItem* currentSelected = ui->patientTree->currentItem();
//   QTreeWidgetItem* currentTopLevelItem = (
// 	  (currentSelected->childCount() == 0 && currentSelected->parent()) ?
// 	    currentSelected->parent() :
// 	    currentSelected
//   );
  
//   if (currentSelected->childCount() > 0) {
// 	SwitchExpandedView(currentTopLevelItem);
//   }

//   // If it's a child item and it got checked
//   if (item->childCount() == 0 && item->checkState(0) == Qt::Checked)
//   {
//     //item->parent()->setCheckState(0, Qt::Checked);
// 	SwitchSubjectAndImage(
// 		item->parent()->data(0, PATIENT_UID_ROLE).toLongLong(),  
// 		item->data(0, IMAGE_PATH_ROLE).toString()
// 	);
//   }
// }

// void MainWindow::ShowTreeContextMenu(const QPoint& pos)
// {
//   qDebug() << QString("Show context menu pressed");

//   if (ui->patientTree->itemAt(pos)) {

//     QMenu *contextMenu = new QMenu(ui->patientTree);
    
//     QAction action1("Remove", this);
//     //action1.setShortcutContext(Qt::ApplicationShortcut);
//     //action1.setShortcut(QKeySequence::Delete);
//     connect(&action1, SIGNAL(triggered()), this, SLOT(TreeContextRemoveItem()));
//     contextMenu->addAction(&action1);
    
//     QAction action2("Set as mask", this);
//     connect(&action2, SIGNAL(triggered()), this, SLOT(TreeContextSetItemAsMask()));

//     if (ui->patientTree->itemAt(pos)->childCount() == 0) {
//       // The item is an image
//       qDebug() << ui->patientTree->itemAt(pos)->data(0, IMAGE_PATH_ROLE);

//       contextMenu->addAction(&action2);
//     }
    
//     contextMenu->exec(ui->patientTree->viewport()->mapToGlobal(pos));
//   }
// }

// void MainWindow::TreeContextRemoveItem()
// {
//   qDebug() << QString("Trying to delete item");

//   if (ui->patientTree->currentItem()->childCount() == 0 &&
// 	  ui->patientTree->currentItem()->parent()->childCount() == 1)
//   {
// 	  delete ui->patientTree->currentItem()->parent();
//   }
//   else {
// 	delete ui->patientTree->currentItem();
//   }

//   if (ui->patientTree->topLevelItemCount() > 0)
//   {
// 	  SwitchExpandedView(ui->patientTree->topLevelItem(0));
//   }
//   //this->ui->stackedWidget->setCurrentIndex(0);
// }

// void MainWindow::TreeContextSetItemAsMask()
// {
//   for (int i = 0; i < ui->patientTree->currentItem()->parent()->childCount(); i++) {
//     ui->patientTree->currentItem()->parent()->child(i)->setText(0, 
//       ui->patientTree->currentItem()->parent()->child(i)->data(0, IMAGE_NAME_ROLE).toString()
//     );
//   }

//   ui->patientTree->currentItem()->setText(0, QString("<Mask>"));
// }

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
