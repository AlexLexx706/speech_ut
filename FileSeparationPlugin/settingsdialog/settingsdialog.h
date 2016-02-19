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
	std::string GetProcessingDir() const { return ui.lineEdit_wavDir->text().toUtf8().data();}
	std::string GetReportsDir() const {return ui.lineEdit_markingDir->text().toUtf8().data();}
	std::string GetOutDir() const {return ui.lineEdit_outDir->text().toUtf8().data();}
	bool MarkingAsUtf8() const {return ui.checkBox_markingAsUtf8->isChecked();}

	int GetMaxBPThreadCount() const {return ui.spinBox_maxBPThreadCount->value();}
	bool UseExternalProcessing() const {return ui.checkBox_useExternalProcessing->isChecked();}

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;

private slots:
	void on_toolButton_setWavDir_clicked();
	void on_toolButton_setMarkingDir_clicked();
	void on_toolButton_setOutDir_clicked();
	void OnAccepted();
	void OnRejected();
};

#endif // SETTINGSDIALOG_H
