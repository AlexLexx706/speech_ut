#ifndef OPENEDFILESDIALOG_H
#define OPENEDFILESDIALOG_H

#include <QDialog>
#include "ui_openedfilesdialog.h"
#include <QStandardItemModel>

class MainWindow;
class OpenedFilesDialog : public QDialog
{
	Q_OBJECT

public:
	OpenedFilesDialog(MainWindow & mw);
	~OpenedFilesDialog();
	void Exec();
private:
	Ui::OpenedFilesDialog ui;
	MainWindow & mw;
	QStandardItemModel model;
private slots:
	void on_tableView_doubleClicked(const QModelIndex & index);
	void on_pushButton_show_clicked();
	//void on_pushButton_open_clicked();
	void on_pushButton_close_clicked();

};

#endif // OPENEDFILESDIALOG_H
