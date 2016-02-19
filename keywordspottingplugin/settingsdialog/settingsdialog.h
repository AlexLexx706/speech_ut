#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"

class QSettings;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QSettings & settings, QWidget *parent = 0);
	~SettingsDialog();
	std::string GetPathToHmm()const {return ui.lineEdit_pathToHmm->text().toUtf8().data();}
	std::string GetPathToDict()const {return ui.lineEdit_pathToDict->text().toUtf8().data();}
	bool IsDictInUtf()const {return ui.checkBox_useUtf8->isChecked();}
	std::string GetPathToTiedls()const {return ui.lineEdit_pathToTiedls->text().toUtf8().data();}
	std::string GetReportsDir()const {return ui.lineEdit_reportsDir->text().toUtf8().data();}
	std::string GetProcessingDir()const {return ui.lineEdit_processingDir->text().toUtf8().data();}
	int GetMaxBPThreadCount() const { return ui.spinBox_maxBPThreadCount->value();}
	std::string GetPathToKeywordsDict() const {return ui.lineEdit_pathToKeywordsDict->text().toUtf8().data();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;

private slots:
	void on_toolButton_setReportsDir_clicked();
	void on_toolButton_setProcessingDir_clicked();
	void OnAccepted();
	void OnRejected();
	void on_toolButton_pathToHmm_clicked();
	void on_toolButton_pathToDict_clicked();
	void on_toolButton_pathToTiedls_clicked();
	void on_toolButton_pathToKeywordsDict_clicked();
};

#endif // SETTINGSDIALOG_H
