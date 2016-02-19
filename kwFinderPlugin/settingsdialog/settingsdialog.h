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
	std::string GetPathToHmm()const {return ui.lineEdit_pathToHmm->text().toUtf8().data();}
	std::string GetPathToDict()const {return ui.lineEdit_pathToDict->text().toUtf8().data();}
	std::string GetPathToTiedls()const {return ui.lineEdit_pathToTiedls->text().toUtf8().data();}
	std::string GetReportsDir()const {return ui.lineEdit_reportsDir->text().toUtf8().data();}
	std::string GetProcessingDir()const {return ui.lineEdit_processingDir->text().toUtf8().data();}
	int GetMaxBPThreadCount() const { return ui.spinBox_maxBPThreadCount->value();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}
	bool IsDictInUtf8() const {return ui.checkBox_useUtf8->isChecked();}

	PRUNING_T GetPruningType()const;
	SP_REAL GetPruningPar()const;
	LOOPACTIONS_T GetLoopActionsType() const;
	bool IsUseLangModel() const;
	int GetNGramSize() const;
	NBEST_T GetNBestType() const {return (NBEST_T)ui.comboBox_NBestType->itemData(ui.comboBox_NBestType->currentIndex()).toInt();}
	int GetNBestCount() const { return ui.spinBox_NBestCount->value();}
	int GetNBestCapacity() const { return ui.spinBox_NBestCapacity->value();}
	bool IsPrintLatticeOutput() const { return ui.checkBox_PrintLatticeOutput->isChecked();}
	std::string GetLatticeOutputFile() const { return ui.lineEdit_LatticeOutputFile->text().toUtf8().data();}
	bool IsUseLatticeOutputDir() const { return ui.checkBox_LatticeOutputDir->isChecked();}
	std::string GetLatticeOutputDir() const { return ui.lineEdit_LatticeOutputDir->text().toUtf8().data();}



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
	void on_toolButton_setLatticeOutputDir_clicked();
};

#endif // SETTINGSDIALOG_H
