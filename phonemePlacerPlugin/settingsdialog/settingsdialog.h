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
	std::string GetHmmModel() const { return ui.lineEdit_hmmModel->text().toUtf8().data();}
	std::string GetTextFileName() const { return ui.lineEdit_textFile->text().toUtf8().data();}
	std::string GetProcessingDir() const {return ui.lineEdit_processingDir->text().toUtf8().data();}
	std::string GetReportsDir() const {return ui.lineEdit_reportsDir->text().toUtf8().data();}
	std::string GetTranscriptionsDir() const {return ui.lineEdit_transcriptionsDir->text().toUtf8().data();}

	bool PhraseIsCorrect() const { return ui.checkBox_phraseIsCorrect->isChecked();}
	int GetMaxBPThreadCount() const {return ui.spinBox_maxBPThreadCount->value();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}
	bool is_output_type_by_states(){return ui.checkBox__output_type->isChecked();}
	bool restartProcessing() const { return ui.checkBox_restartProcessing->isChecked();}

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;

private slots:
	void on_toolButton_settranscriptionsDir_clicked();
	void on_toolButton_setReportsDir_clicked();
	void on_toolButton_setProcessingDir_clicked();
	void on_toolButton_setTextFile_clicked();
	void on_toolButton_setHmmModel_clicked();
	void OnAccepted();
	void OnRejected();
};

#endif // SETTINGSDIALOG_H
