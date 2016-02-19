#ifndef REMOVEPHRASESBYNAMEDIALOG_H
#define REMOVEPHRASESBYNAMEDIALOG_H

#include <QDialog>
#include "ui_removephrasesbynamedialog.h"

class QSettings;

class RemovePhrasesByNameDialog : public QDialog
{
	Q_OBJECT

public:
	RemovePhrasesByNameDialog(QSettings & settings, QWidget *parent = 0);
	~RemovePhrasesByNameDialog();
	QString GetPhrasesName() const {return ui.lineEdit_phrasesName->text();}
	bool IsRemoveOnlyPhrases() const {return ui.radioButton_removeOnlyPhrases->isChecked();}
	bool IsRemovePhrasesAndSignal() const {return ui.radioButton_removePhrasesAndSignal->isChecked();}
	bool IsCaseSensitive() const {return ui.checkBox_caseSensitive->isChecked();}

private:
	Ui::RemovePhrasesByNameDialogClass ui;
	QSettings & settings;
private slots:
	void on_accepted();
	void on_rejected();
};

#endif // REMOVEPHRASESBYNAMEDIALOG_H
