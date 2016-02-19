#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>


SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);
	settings.beginGroup("FileSeparationPlugin");

	ui.lineEdit_wavDir->setText(settings.value("wavDir", "FileSeparationPlugin_testData/wav").toString());
	ui.lineEdit_markingDir->setText(settings.value("markingDir", "FileSeparationPlugin_testData/marking").toString());
	ui.lineEdit_outDir->setText(settings.value("outDir", "FileSeparationPlugin_testData/out").toString());
	ui.checkBox_markingAsUtf8->setChecked(settings.value("markingAsUtf8", false).toBool());


	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());


	settings.endGroup();

	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::OnAccepted()
{
	settings.beginGroup("FileSeparationPlugin");

	settings.setValue("wavDir", ui.lineEdit_wavDir->text());
	settings.setValue("markingDir", ui.lineEdit_markingDir->text());
	settings.setValue("outDir",ui.lineEdit_outDir->text());
	settings.setValue("markingAsUtf8", ui.checkBox_markingAsUtf8->isChecked());

	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());

	settings.endGroup();
}


void SettingsDialog::OnRejected()
{
	settings.beginGroup("FileSeparationPlugin");

	ui.lineEdit_wavDir->setText(settings.value("wavDir", "FileSeparationPlugin_testData/wav").toString());
	ui.lineEdit_markingDir->setText(settings.value("markingDir", "FileSeparationPlugin_testData/marking").toString());
	ui.lineEdit_outDir->setText(settings.value("outDir", "FileSeparationPlugin_testData/out").toString());
	ui.checkBox_markingAsUtf8->setChecked(settings.value("markingAsUtf8", false).toBool());


	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	settings.endGroup();
}

void SettingsDialog::on_toolButton_setWavDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку со звуковыми файлами"),
												ui.lineEdit_wavDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_wavDir->setText(dir);
}

void SettingsDialog::on_toolButton_setMarkingDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с разметками"),
												ui.lineEdit_markingDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_markingDir->setText(dir);
}

void SettingsDialog::on_toolButton_setOutDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с разметками"),
												ui.lineEdit_outDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_outDir->setText(dir);
}