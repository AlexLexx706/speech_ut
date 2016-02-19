#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"
#include <Voice Activity Detection/VAD_GMM.h>
#include "../json_spirit/json_spirit.h"
class QSettings;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QSettings & settings, QWidget *parent = 0);
	~SettingsDialog();

	std::string GetProcessingDir() const {return ui.lineEdit_processingDir->text().toUtf8().data();}
	std::string GetReportsDir() const {return ui.lineEdit_reportsDir->text().toUtf8().data();}
	std::string GetModiffedFilesDir() const {return ui.lineEdit_modiffedFilesDir->text().toUtf8().data();}
	bool IsCutNoice() const {return ui.checkBox_cutNoice->isChecked();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}
	int GetMaxBPThreadCount() const {return ui.spinBox_maxBPThreadCount->value();}
	std::string GetVadParams(){ return vadPrams;};

	std::string GetUBMModelPath() const { return ui.lineEdit_umbModel_dc->text().toUtf8().data();}
	std::string GetPauseDCModelPath() const { return ui.lineEdit_pauseModel_dc->text().toUtf8().data();}
	std::string GetSpeechDCModelPath() const { return ui.lineEdit_speechModel_dc->text().toUtf8().data();}
	bool IsUseDictorCount() const { return ui.groupBox_dc->isChecked();}
	double GetThresholdDC() const { return ui.doubleSpinBox_tresholdParam_dc->value();}
	int GetLargeWinSize() const { return ui.spinBox_largeWinSize->value();}
	bool restartProcessing() const { return ui.checkBox_restartProcessing->isChecked();}
	json_spirit::mObject GetCutModelsDesc() const;

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;
	std::string vadPrams;

	void Init();
	void UpdateCutModelsList(const QString & models_dir,
							bool use_cut_models,
							const QMap<QString, bool> & selected_state);
	QPair<bool, QMap<QString, bool> > GetCutModelsDescInternal() const;
private slots:
	void on_toolButton_setModiffedFilesDir_clicked();
	void on_toolButton_setReportsDir_clicked();
	void on_toolButton_setProcessingDir_clicked();
	void on_toolButton_lineEdit_noSpeechModel_clicked();
	void OnRejected();
	void OnAccepted();
	void on_toolButton_setPauseModel_clicked();
	void on_toolButton_SetSpeechModel_clicked();
	void on_pushButton_ok_clicked();
	void on_pushButton_cancel_clicked();
	void on_toolButton_addToSpeechList_clicked();
	void on_toolButton_delFromSpeechList_clicked();
	void on_toolButton_addToSeparatorList_clicked();
	void on_toolButton_delFromSeparatorList_clicked();

	void on_toolButton_ubmModel_dc_clicked();
	void on_toolButton_speechModel_dc_clicked();
	void on_toolButton_pauseModel_dc_clicked();
	void on_toolButton_update_cut_model_clicked();
};

#endif // SETTINGSDIALOG_H
