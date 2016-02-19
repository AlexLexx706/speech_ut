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
	std::string GetProcessingDir() const {return ui.lineEdit_processingDir->text().toUtf8().data();}
	std::string GetReportsDir() const {return ui.lineEdit_reportsDir->text().toUtf8().data();}
	int GetMaxBPThreadCount() const {return ui.spinBox_maxBPThreadCount->value();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}
	int GetStartThreshold() const { return ui.spinBox_Start_Threshold->value();}
	int GetEndThreshold() const { return ui.spinBox_End_Treshold->value();}
	bool ShowBest() const { return ui.checkBox_showBest->isChecked();}
	std::string GetLatticeFile() const { return ui.lineEdit_latticeFile->text().toUtf8().data();}
	bool UsePostProcessing() const { return ui.checkBox_usePostProcessing->isChecked();}

	QMap<QString, QString> GetParameters() const;
	std::string GetProgrammFileName() const {return ui.lineEdit_programmFileName->text().toUtf8().data();}


private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;

private slots:
	void on_toolButton_setReportsDir_clicked();
	void on_toolButton_setProcessingDir_clicked();
	void OnAccepted();
	void OnRejected();
	void on_pushButton_addParam_clicked();
	void on_pushButton_delParam_clicked();
};

#endif // SETTINGSDIALOG_H
