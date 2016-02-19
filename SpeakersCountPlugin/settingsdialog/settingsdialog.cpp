#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include "../json_spirit/json_spirit.h"
#include <QDebug>

SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);

	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));
	
	Init();
}

void SettingsDialog::Init()
{
	settings.beginGroup("SpeakersCountPlugin");

	ui.lineEdit_processingDir->setText(settings.value("processingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("reportDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	ui.lineEdit_umbModel->setText(settings.value("umbModel", "speakerscountdata\\models\\ubm.gmm").toString());
	ui.lineEdit_speechModel->setText(settings.value("speechModel", "speakerscountdata\\models\\speech.gmm").toString());
	ui.lineEdit_pauseModel->setText(settings.value("pauseModel", "speakerscountdata\\models\\pause.gmm").toString());
	ui.doubleSpinBox_tresholdParam->setValue(settings.value("tresholdParam", 0.0071).toDouble());


	settings.endGroup();
}

SettingsDialog::~SettingsDialog()
{
}


void SettingsDialog::OnAccepted()
{

	settings.beginGroup("SpeakersCountPlugin");

	settings.setValue("processingDir", ui.lineEdit_processingDir->text());
	settings.setValue("reportDir", ui.lineEdit_reportsDir->text());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());

	settings.setValue("umbModel", ui.lineEdit_umbModel->text());
	settings.setValue("speechModel", ui.lineEdit_speechModel->text());
	settings.setValue("pauseModel", ui.lineEdit_pauseModel->text());
	settings.setValue("tresholdParam", ui.doubleSpinBox_tresholdParam->value());

	settings.endGroup();
}


void SettingsDialog::OnRejected()
{
	Init();
}

void SettingsDialog::on_toolButton_ubmModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл модели umb"),
												 "",
												 QString::fromLocal8Bit("Модель (*.gmm)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_umbModel->setText(fileName);
}

void SettingsDialog::on_toolButton_setPauseModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл модели паузы"),
												 "",
												 QString::fromLocal8Bit("Модель (*.gmm)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_pauseModel->setText(fileName);
}

void SettingsDialog::on_toolButton_SetSpeechModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл модели речи"),
												 "",
												 QString::fromLocal8Bit("Модель (*.gmm)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_speechModel->setText(fileName);
}

void SettingsDialog::on_pushButton_ok_clicked()
{
	accept();
}

void SettingsDialog::on_pushButton_cancel_clicked()
{
	reject();
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
