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

	settings.beginGroup("keywordSpottingPlugin");

	ui.lineEdit_pathToHmm->setText(settings.value("pathToHmm", "keywordSpottingPluginData/hmmdefs").toString());
	ui.lineEdit_pathToDict->setText(settings.value("pathToDict", "keywordSpottingPluginData/dictUsed.txt").toString());
	ui.checkBox_useUtf8->setChecked(settings.value("dictInUtf8", false).toBool());

	ui.lineEdit_pathToTiedls->setText(settings.value("pathToTiedls", "keywordSpottingPluginData/tiedls.txt").toString());
	ui.lineEdit_pathToKeywordsDict->setText(settings.value("pathToKeywordsDict", "keywordSpottingPluginData/keywords.txt").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());

	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	settings.endGroup();

}

SettingsDialog::~SettingsDialog()
{

}


void SettingsDialog::OnAccepted()
{
	settings.beginGroup("keywordSpottingPlugin");

	settings.setValue("pathToHmm", ui.lineEdit_pathToHmm->text());
	settings.setValue("pathToDict", ui.lineEdit_pathToDict->text());
	settings.setValue("dictInUtf8", ui.checkBox_useUtf8->isChecked());

	settings.setValue("pathToTiedls", ui.lineEdit_pathToTiedls->text());
	settings.setValue("pathToKeywordsDict", ui.lineEdit_pathToKeywordsDict->text());

	settings.setValue("ProcessingDir", ui.lineEdit_processingDir->text());
	settings.setValue("ReportsDir", ui.lineEdit_reportsDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());

	settings.endGroup();
}

void SettingsDialog::OnRejected()
{
	settings.beginGroup("keywordSpottingPlugin");

	ui.lineEdit_pathToHmm->setText(settings.value("pathToHmm", "keywordSpottingPluginData/hmmdefs").toString());
	ui.lineEdit_pathToDict->setText(settings.value("pathToDict", "keywordSpottingPluginData/dictUsed.txt").toString());
	ui.checkBox_useUtf8->setChecked(settings.value("dictInUtf8", false).toBool());

	ui.lineEdit_pathToTiedls->setText(settings.value("pathToTiedls", "keywordSpottingPluginData/tiedls.txt").toString());
	ui.lineEdit_pathToKeywordsDict->setText(settings.value("pathToKeywordsDict", "keywordSpottingPluginData/keywords.txt").toString());
	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	settings.endGroup();
}

void SettingsDialog::on_toolButton_pathToHmm_clicked()
{
	 QString fileName = QFileDialog::getOpenFileName(this, tr("Open File Hmm"),
													 ui.lineEdit_pathToHmm->text());
	 if ( !fileName.isEmpty() )
		 ui.lineEdit_pathToHmm->setText(fileName);
}

void SettingsDialog::on_toolButton_pathToDict_clicked()
{
	 QString fileName = QFileDialog::getOpenFileName(this, tr("Open File Dict"),
													 ui.lineEdit_pathToDict->text());
	 if ( !fileName.isEmpty() )
		 ui.lineEdit_pathToDict->setText(fileName);
}

void SettingsDialog::on_toolButton_pathToTiedls_clicked()
{
	 QString fileName = QFileDialog::getOpenFileName(this, tr("Open File Tiedls"),
													 ui.lineEdit_pathToTiedls->text());
	 if ( !fileName.isEmpty() )
		 ui.lineEdit_pathToTiedls->setText(fileName);
}

void SettingsDialog::on_toolButton_pathToKeywordsDict_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Установить список ключевых слов."),
													 ui.lineEdit_pathToKeywordsDict->text());
	 if ( !fileName.isEmpty() )
		 ui.lineEdit_pathToKeywordsDict->setText(fileName);
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