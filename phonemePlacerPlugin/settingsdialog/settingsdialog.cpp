#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>


SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);
	OnRejected();
	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));
}

SettingsDialog::~SettingsDialog()
{

}


void SettingsDialog::on_toolButton_setHmmModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл модели HMM"),
									ui.lineEdit_hmmModel->text());
	if ( !fileName.isEmpty() )
		ui.lineEdit_hmmModel->setText(fileName);
}

void SettingsDialog::on_toolButton_setTextFile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл текста"),
												 ui.lineEdit_textFile->text(),
												 QString::fromLocal8Bit("Модель (*.txt *.lab)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_textFile->setText(fileName);
}

void SettingsDialog::OnAccepted()
{
	settings.beginGroup("PhonemePlacerPlugin");

	settings.setValue("restartProcessing", ui.checkBox_restartProcessing->isChecked());

	settings.setValue("hmmModel", ui.lineEdit_hmmModel->text());
	settings.setValue("textFile", ui.lineEdit_textFile->text());
	settings.setValue("phraseIsCorrect",ui.checkBox_phraseIsCorrect->isChecked());
	settings.setValue("ProcessingDir", ui.lineEdit_processingDir->text());
	settings.setValue("ReportsDir", ui.lineEdit_reportsDir->text());
	settings.setValue("TranscriptionsDir", ui.lineEdit_transcriptionsDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());
	settings.setValue("output_type_by_state", ui.checkBox__output_type->isChecked());

	settings.endGroup();
}


void SettingsDialog::OnRejected()
{
	settings.beginGroup("PhonemePlacerPlugin");

	ui.checkBox_restartProcessing->setChecked(settings.value("restartProcessing", true).toBool());
	ui.lineEdit_hmmModel->setText(settings.value("hmmModel", "phonemeplacerplugindata/model/hmmdefs_RU_PLP").toString());
	ui.lineEdit_textFile->setText(settings.value("textFile", "phonemeplacerplugindata/test.txt").toString());
	ui.checkBox_phraseIsCorrect->setChecked(settings.value("phraseIsCorrect", false).toBool());
	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.lineEdit_transcriptionsDir->setText(settings.value("TranscriptionsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());
	ui.checkBox__output_type->setChecked(settings.value("output_type_by_state", false).toBool());

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

void SettingsDialog::on_toolButton_settranscriptionsDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку транскрипций"),
												ui.lineEdit_transcriptionsDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_transcriptionsDir->setText(dir);
}