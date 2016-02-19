#include "removephrasesbynamedialog.h"
#include <QSettings>

RemovePhrasesByNameDialog::RemovePhrasesByNameDialog(QSettings & _settings, QWidget *parent): 
	QDialog(parent), 
	settings(_settings)
{
	ui.setupUi(this);
	connect(this, SIGNAL(accepted()), this, SLOT(on_accepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(on_rejected()));


	settings.beginGroup("RemovePhrasesByNameDialog");

	ui.lineEdit_phrasesName->setText(settings.value("phrasesName", "noise").toString());
	ui.radioButton_removeOnlyPhrases->setChecked(settings.value("removeOnlyPhrases", false).toBool());
	ui.radioButton_removePhrasesAndSignal->setChecked(settings.value("removePhrasesAndSignal", true).toBool());
	ui.checkBox_caseSensitive->setChecked(settings.value("caseSensitive", true).toBool());

	settings.endGroup();
}

RemovePhrasesByNameDialog::~RemovePhrasesByNameDialog()
{}

void RemovePhrasesByNameDialog::on_accepted()
{
	settings.beginGroup("RemovePhrasesByNameDialog");

	settings.setValue("phrasesName", ui.lineEdit_phrasesName->text());
	settings.setValue("removeOnlyPhrases", ui.radioButton_removeOnlyPhrases->isChecked());
	settings.setValue("removePhrasesAndSignal", ui.radioButton_removePhrasesAndSignal->isChecked());
	settings.setValue("caseSensitive", ui.checkBox_caseSensitive->isChecked());

	settings.endGroup();
}

void RemovePhrasesByNameDialog::on_rejected()
{
	settings.beginGroup("RemovePhrasesByNameDialog");

	ui.lineEdit_phrasesName->setText(settings.value("phrasesName", "noise").toString());
	ui.radioButton_removeOnlyPhrases->setChecked(settings.value("removeOnlyPhrases", false).toBool());
	ui.radioButton_removePhrasesAndSignal->setChecked(settings.value("removePhrasesAndSignal", true).toBool());
	ui.checkBox_caseSensitive->setChecked(settings.value("caseSensitive", true).toBool());

	settings.endGroup();
}
