#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>

SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);
	connect(this, SIGNAL(accepted()),this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()),this, SLOT(OnRejected()));

	settings.beginGroup("RecognitonPlugin");

	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());
	
	ui.lineEdit_pauseModel->setText(settings.value("pauseModel", "recognitionPluginData/VADmodels/_pause.mdl").toString());
	ui.lineEdit_speechModel->setText(settings.value("speechModel", "recognitionPluginData/VADmodels/_speech.mdl").toString());
	ui.lineEdit_noiceModels->setText(settings.value("noiceModels", "recognitionPluginData/VADmodels").toString());
	ui.lineEdit_languagesModels->setText(settings.value("languagesModels","recognitionPluginData/TestModels/Language Models").toString());
	ui.lineEdit_phonemModels->setText(settings.value("phonemModels","recognitionPluginData/TestModels/Phoneme models").toString());


	settings.endGroup();
}

SettingsDialog::~SettingsDialog()
{
}


void SettingsDialog::OnAccepted()
{
	settings.beginGroup("RecognitonPlugin");

	settings.setValue("ProcessingDir", ui.lineEdit_processingDir->text());
	settings.setValue("ReportsDir", ui.lineEdit_reportsDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());

	settings.setValue("pauseModel", ui.lineEdit_pauseModel->text());
	settings.setValue("speechModel", ui.lineEdit_speechModel->text());
	settings.setValue("noiceModels", ui.lineEdit_noiceModels->text());
	settings.setValue("languagesModels", ui.lineEdit_languagesModels->text());
	settings.setValue("phonemModels", ui.lineEdit_phonemModels->text());

	settings.endGroup();
}

void SettingsDialog::OnRejected()
{
	settings.beginGroup("RecognitonPlugin");

	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	ui.lineEdit_pauseModel->setText(settings.value("pauseModel", "recognitionPluginData/VADmodels/_pause.mdl").toString());
	ui.lineEdit_speechModel->setText(settings.value("speechModel", "recognitionPluginData/VADmodels/_speech.mdl").toString());
	ui.lineEdit_noiceModels->setText(settings.value("noiceModels", "recognitionPluginData/VADmodels").toString());
	ui.lineEdit_languagesModels->setText(settings.value("languagesModels","recognitionPluginData/TestModels/Language Models").toString());
	ui.lineEdit_phonemModels->setText(settings.value("phonemModels","recognitionPluginData/TestModels/Phoneme models").toString());

	settings.endGroup();

}

void SettingsDialog::on_toolButton_setProcessingDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку для обработки"),
												ui.lineEdit_processingDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_processingDir->setText(dir);

}

void SettingsDialog::on_toolButton_setReportsDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку для отчётов"),
												ui.lineEdit_reportsDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_reportsDir->setText(dir);
}

void SettingsDialog::on_toolButton_setPauseModel_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Установить модель паузы"),
												ui.lineEdit_pauseModel->text(),
												"Model (*.mdl)");

	if ( !file.isEmpty() )
		ui.lineEdit_pauseModel->setText(file);
}

void SettingsDialog::on_toolButton_setSpeechModel_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Установить модель речи"),
												ui.lineEdit_pauseModel->text(),
												"Model (*.mdl)");

	if ( !file.isEmpty() )
		ui.lineEdit_pauseModel->setText(file);
}

void SettingsDialog::on_toolButton_setNoiceModes_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с моделями шума"),
												ui.lineEdit_noiceModels->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_noiceModels->setText(dir);

}

void SettingsDialog::on_toolButton_setLanguagesModels_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с моделями языка"),
												ui.lineEdit_languagesModels->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_languagesModels->setText(dir);
}

void SettingsDialog::on_toolButton_setPhonemModels_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с моделями фонем"),
												ui.lineEdit_phonemModels->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_phonemModels->setText(dir);
}
