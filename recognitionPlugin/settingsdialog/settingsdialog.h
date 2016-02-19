#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"
#include <Viterbi/engine.h>

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

	std::string GetPauseModel() const {return ui.lineEdit_pauseModel->text().toUtf8().data();}
	std::string GetSpeechModel() const {return ui.lineEdit_speechModel->text().toUtf8().data();}
	std::string GetNoiceModels() const {return ui.lineEdit_noiceModels->text().toUtf8().data();}
	std::string GetLanguagesModels() const {return ui.lineEdit_languagesModels->text().toUtf8().data();}
	std::string GetPhonemModels() const {return ui.lineEdit_phonemModels->text().toUtf8().data();}


private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;

private slots:
	void on_toolButton_setReportsDir_clicked();
	void on_toolButton_setProcessingDir_clicked();
	
	void on_toolButton_setPauseModel_clicked();
	void on_toolButton_setSpeechModel_clicked();
	void on_toolButton_setNoiceModes_clicked();
	void on_toolButton_setLanguagesModels_clicked();
	void on_toolButton_setPhonemModels_clicked();

	void OnAccepted();
	void OnRejected();
};

#endif // SETTINGSDIALOG_H
