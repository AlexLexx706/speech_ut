#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"
#include <Voice Activity Detection/VAD_GMM.h>
class QSettings;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QSettings & settings, QWidget *parent = 0);
	~SettingsDialog();

	std::string GetProcessingDir() const {return ui.lineEdit_processingDir->text().toUtf8().data();}
	std::string GetReportsDir() const {return ui.lineEdit_reportsDir->text().toUtf8().data();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}
	int GetMaxBPThreadCount() const {return ui.spinBox_maxBPThreadCount->value();}

	std::string GetUbmModelPath() const { return ui.lineEdit_umbModel->text().toUtf8().data();}
	std::string GetSpeechModelPath() const { return ui.lineEdit_speechModel->text().toUtf8().data();}
	std::string GetPauseModelPath() const { return ui.lineEdit_pauseModel->text().toUtf8().data();}
	double GetTresholdParam() const { return ui.doubleSpinBox_tresholdParam->value();}

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;
	std::string vadPrams;

	void Init();

private slots:
	void on_toolButton_setReportsDir_clicked();
	void on_toolButton_setProcessingDir_clicked();
	void OnRejected();
	void OnAccepted();
	void on_toolButton_ubmModel_clicked();
	void on_toolButton_setPauseModel_clicked();
	void on_toolButton_SetSpeechModel_clicked();
	void on_pushButton_ok_clicked();
	void on_pushButton_cancel_clicked();
};

#endif // SETTINGSDIALOG_H
