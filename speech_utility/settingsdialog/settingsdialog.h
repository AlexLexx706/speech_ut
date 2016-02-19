#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"

class MainWindow;
class QSettings;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(MainWindow * parent);
	~SettingsDialog();
	bool IsPlayAfterChangePhrase() const {return ui.checkBox_playAfterChangePhrase->isChecked();}
	bool UseOffsetAfterPause() const {return ui.checkBox_pauseOffset->isChecked();}
	int GetOffsetAfterPause() const {return ui.spinBox_pauseOffset->value();}
	bool IsCopyPhraseTextAC() const {return ui.checkBox_copyPhraseTextAC->isChecked();}
	bool IsSavePhraseTextWE() const {return ui.checkBox_savePhraseText->isChecked();}
	bool IsAutoScalePlot() const {return ui.checkBox_autoScalePlot->isChecked();}
	qreal GetPlaySpeed() const { return ui.doubleSpinBox_playSpeed->value();}
	bool IsAutoMoveMarker() const { return ui.checkBox_autoMoveMarker->isChecked();}
	bool IsPlayPosTracking() const { return ui.checkBox_playPosTracking->isChecked();}
	bool IsCheckPhrasesDuratio() const { return ui.checkBox_checkPhrasesDuraton->isChecked();}
	QString GetPhrasesName() const { return ui.lineEdit_phraseName->text();}
	int GetPhraseDurationThreshold() const { return ui.spinBox_phraseDurationThreshold->value(); }
	bool IsEditPhraseNameInMarking() const { return ui.checkBox_editPhraseNameInMarking->isChecked();}
	bool ShowPluginMassages(){return ui.checkBox_showPluginMassages->isChecked();};
	bool OpenMarkingSameWave(){return ui.checkBox_openMarkingAsWave->isChecked();}
	bool OpenMarkingAsUTF8(){return ui.checkBox_openMarkingAsUTF8->isChecked();}
	QFont GetMarkingFont() const {return markingFont;}
	bool IsAutoSaveMarking() const {return ui.checkBox_autoSaveMarking->isChecked();}
	bool IsAutoSaveSignalDesc() const {return ui.checkBox_autoSaveSignalDesc->isChecked();}
	bool IsShowSignalDescInStartup() const {return ui.checkBox_show_signal_desc_in_startup->isChecked();}

private slots:
	//void on_listWidget_actionList_currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous);
	//void on_pushButton_setShortCut_clicked();
	void on_tableWidget_actionList_itemChanged(QTableWidgetItem * item);
	void OnAccepted();
	void OnRejected();
	void on_pushButton_setFont_clicked();
	void on_pushButton_setMarkingFont_clicked();

private:
	void LoadShortCuts();
	void ResetShortcut(const QObject * obj, const QString & key);

private:
	Ui::SettingsDialogClass ui;
	QSettings & settings;
	QFont font;
	QFont markingFont;
};

#endif // SETTINGSDIALOG_H
