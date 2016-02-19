#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"

class QSettings;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	typedef QString ChannelNameStr;
	typedef QString LangNameStr;
	typedef QString RecNameStr;

	SettingsDialog(QSettings & settings, QWidget *parent = 0);
	~SettingsDialog();

	bool UseMoreServer() const {return ui.groupBox_moreServer->isChecked();}
	std::string MoreServerHost() const {return ui.lineEdit_moreServerHost->text().toUtf8().data();}
	int MoreServerPort() const {return ui.spinBox_moreServerPort->value();}
	
	std::string GetPauseModelPath() const {return ui.lineEdit_pauseModelPath->text().toUtf8().data();}
	std::string GetSpeechModelPath() const {return ui.lineEdit_speechModelPath->text().toUtf8().data();}
	std::string GetModelsDir() const {return ui.lineEdit_dictorsModelsDir->text().toUtf8().data();}
	std::string GetModelsMask() const { return ui.lineEdit_modelsMask->text().toUtf8().data();}
	bool RestartProcessing() const { return ui.checkBox_restartProcessing->isChecked();}

	std::string GetNGrammModelsPath() const {return ui.lineEdit_nGrammModelsDir->text().toUtf8().data();}
	std::string GetNGrammModelsMask() const { return ui.lineEdit_nGrammModelsMask->text().toUtf8().data();}
	std::string GetNGrammModelsMaskAux() const { return ui.lineEdit_nGrammModelsMask_aux->text().toUtf8().data();}

	std::string GetPhonemsModelsPath() const {return ui.lineEdit_phonemsModelsDir->text().toUtf8().data();}
	std::string GetPhonemsModelsMask() const {return ui.lineEdit_phonemsModelsMask->text().toUtf8().data();}
	QPair<QString, QStringList> GetCurLanguages() const {return ui.treeWidget_nGrammModels->GetCurModels();}
	QPair<QString, QStringList> GetPhonems() const {return ui.treeWidget_phonemModels->GetCurModels();}


	std::string GetProcessingDir() const { return ui.lineEdit_wavDir->text().toUtf8().data();}
	std::string GetOutDir() const {return ui.lineEdit_outDir->text().toUtf8().data();}
	int GetMaxBPThreadCount() const {return ui.spinBox_maxBPThreadCount->value();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}

	bool SaveRecognitionReport() const {return ui.checkBox_saveRecognitionReport->isChecked();}
	bool SortFilesByLangs() const {return ui.checkBox_sortFilesByLangs->isChecked();}
	bool CreateLabFile() const {return ui.checkBox_createLabFile->isChecked();}
	bool UseVad() const {return ui.groupBox_vad->isChecked();}
	bool useTxtModel() const {return ui.comboBox_ngrammModelType->itemData(ui.comboBox_ngrammModelType->currentIndex()).toInt() == 0;}
	int ngrammCount() const {return ui.spinBox_ngrammCount->value();}
	double threshold() const {return ui.doubleSpinBox_threshold->value();}
	bool noPhonemeChannels() const {return ui.checkBox_noPhonemChannels->isChecked();} 
	bool useOpenTask() const {return ui.comboBox_taskType->itemData(ui.comboBox_taskType->currentIndex()).toInt() == 0;}

	static bool LoadNGrammModels(const QString & path, 
						const QString & mask,
						const QString & mask_aux,
						QMap<QString, QStringList> & out,
						QString & errorDesc,
						int clientID = -1,
						bool loadModelInRecognition = false,
						bool txtModel = true);
	
	static bool LoadPhonemsModels(bool noChannels,
							const QString & dir,
							const QString & mask,
							QMap<QString, QStringList> & out,
							QString & errorDesc,
							int clientID = -1,
							bool loadModelInRecognition = false);

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;
	int threadsCount;

private slots:
	void OnAccepted();
	void OnRejected();
	
	void on_toolButton_setPauseModelPath_clicked();
	void on_toolButton_setSpeechModelPath_clicked();
	void on_toolButton_setDictorsModelsDir_clicked();
	void on_toolButton_setNGrammModelsDir_clicked();
	void on_toolButton_setPhonemsModelsDir_clicked();
	void on_toolButton_setWavDir_clicked();
	void on_toolButton_setOutDir_clicked();
	void on_toolButton_checkNgarmms_clicked();
	void on_toolButton_checkPhonems_clicked();
	void on_checkBox_useExternalProcessing_clicked(bool checked);
};

#endif // SETTINGSDIALOG_H
