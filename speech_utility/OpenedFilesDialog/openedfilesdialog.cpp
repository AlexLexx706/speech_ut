#include "openedfilesdialog.h"
#include "../mainwindow.h"
#include <QFileInfo>
#include "../fileviewcontroller/fileviewcontroller.h"
#include "../fileview/fileview.h"
#include <QItemSelectionModel>
#include <QMessageBox>

OpenedFilesDialog::OpenedFilesDialog(MainWindow & _mw):
	mw(_mw)
{
	ui.setupUi(this);
}

OpenedFilesDialog::~OpenedFilesDialog()
{

}

void OpenedFilesDialog::Exec()
{
	model.clear();
	model.setHorizontalHeaderLabels(QStringList() << QObject::trUtf8("Имя")<< QObject::trUtf8("Путь"));
	QModelIndex cur_index;
	
	for ( int i= 0; i < mw.GetFVC().count(); i++ )
	{
		FileView * fw = dynamic_cast<FileView *>(mw.GetFVC().widget(i));

		QFileInfo fi(fw->GetFileName());
		
		QStandardItem * item1 = new QStandardItem(fi.fileName());
		item1->setData((int)fw);
		item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		QStandardItem * item2 = new QStandardItem(fi.filePath());
		item2->setData((int)fw);
		item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		model.appendRow(QList<QStandardItem *>() << item1 << item2);

		if ( fw == mw.GetFVC().GetCurFileView())
			cur_index = model.indexFromItem(item1);
	}

	ui.tableView->setModel(&model);
	ui.tableView->setCurrentIndex(cur_index);
	exec();
}

void OpenedFilesDialog::on_tableView_doubleClicked(const QModelIndex & index)
{
	if ( QStandardItem * item = model.itemFromIndex(index) )
	{
		mw.GetFVC().setCurrentWidget((QWidget *)item->data().toInt());
		accept();
	}
}

void OpenedFilesDialog::on_pushButton_show_clicked()
{
	QModelIndex index = ui.tableView->currentIndex();
	if ( index.isValid() )
		mw.GetFVC().setCurrentWidget((QWidget *)model.itemFromIndex(index)->data().toInt());
}

//void OpenedFilesDialog::on_pushButton_open_clicked()
//{
//
//}

void OpenedFilesDialog::on_pushButton_close_clicked()
{
	if( QItemSelectionModel * sm = ui.tableView->selectionModel() )
	{
		QModelIndexList mil = sm->selectedRows(0);
		if ( !mil.empty() && 
			QMessageBox::question(this, QObject::trUtf8("Внимание!"), 
					QObject::trUtf8("Выделенные файлы будут закрыты, все данные будут утерянны, вы готовы к такому повороту событий?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes )
		{
			qSort(mil.begin(), mil.end());

			for ( int i = mil.size() -1 ; i >= 0; i--)
			{
				mw.GetFVC().DelFileView((FileView *)model.itemFromIndex(mil[i])->data().toInt());
				int row = mil[i].row();
				model.removeRows(row,1);
			}
		}
	}
}

