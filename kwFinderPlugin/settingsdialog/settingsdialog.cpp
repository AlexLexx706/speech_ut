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

	settings.beginGroup("kwFinderPlugin");

	ui.lineEdit_pathToHmm->setText(settings.value("pathToHmm", "kwFinderPluginData/hmmdefs").toString());
	ui.lineEdit_pathToDict->setText(settings.value("pathToDict", "kwFinderPluginData/dictUsed.txt").toString());
	ui.checkBox_useUtf8->setChecked(settings.value("dictInUtf8", false).toBool());
	ui.lineEdit_pathToTiedls->setText(settings.value("pathToTiedls", "kwFinderPluginData/tiedls.txt").toString());

	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());

	ui.comboBox_PruningType->clear();
	ui.comboBox_PruningType->addItem("NO_PRUN", (int)NO_PRUN);
	ui.comboBox_PruningType->addItem("PATHNUM", (int)PATHNUM);
	ui.comboBox_PruningType->addItem("PATHPROB", (int)PATHPROB);
	ui.comboBox_PruningType->addItem("PATHPRATIO", (int)PATHPRATIO);
	int index = ui.comboBox_PruningType->findText(settings.value("PruningType", "NO_PRUN").toString());
	ui.comboBox_PruningType->setCurrentIndex(index == -1 ? 0: index); 

	ui.doubleSpinBox_PruningPar->setValue(settings.value("PruningPar",0.).toDouble());

	ui.comboBox_LoopActionsType->clear();
	ui.comboBox_LoopActionsType->addItem("NO_LOOP", (int)NO_LOOP);
	ui.comboBox_LoopActionsType->addItem("LOOP_RECORD", (int)LOOP_RECORD);
	ui.comboBox_LoopActionsType->addItem("LOOP_NO_RECORD", (int)LOOP_NO_RECORD);
	index = ui.comboBox_LoopActionsType->findText(settings.value("LoopActionsType", "LOOP_RECORD").toString());
	ui.comboBox_LoopActionsType->setCurrentIndex(index == -1 ? 0 : index);

	ui.checkBox_UseLangModel->setChecked(settings.value("UseLangModel", false).toBool());
	ui.spinBox_NGramSize->setValue(settings.value("NGramSize", 0).toInt());

	ui.comboBox_NBestType->clear();
	ui.comboBox_NBestType->addItem("NO_NBEST", (int)NO_NBEST);
	ui.comboBox_NBestType->addItem("LATTICE", (int)LATTICE);
	ui.comboBox_NBestType->addItem("WORD_DEPENDENT", (int)WORD_DEPENDENT);
	index = ui.comboBox_NBestType->findText(settings.value("NBestType", "NO_NBEST").toString());
	ui.comboBox_NBestType->setCurrentIndex(index == -1? 0 : index);

	ui.spinBox_NBestCount->setValue(settings.value("NBestCount", 0).toInt());
	ui.spinBox_NBestCapacity->setValue(settings.value("NBestCapacity", 0).toInt());
	ui.checkBox_PrintLatticeOutput->setChecked(settings.value("PrintLatticeOutput", false).toBool());
	ui.lineEdit_LatticeOutputFile->setText(settings.value("LatticeOutputFile", "").toString());

	ui.checkBox_LatticeOutputDir->setChecked(settings.value("UseLatticeOutputDir", false).toBool());
	ui.lineEdit_LatticeOutputDir->setText(settings.value("LatticeOutputDir", "").toString());


	settings.endGroup();

}

SettingsDialog::~SettingsDialog()
{

}

PRUNING_T SettingsDialog::GetPruningType()const
{
	return (PRUNING_T)ui.comboBox_PruningType->itemData(ui.comboBox_PruningType->currentIndex()).toInt();
}

SP_REAL SettingsDialog::GetPruningPar()const
{
	return ui.doubleSpinBox_PruningPar->value();
}

LOOPACTIONS_T SettingsDialog::GetLoopActionsType() const
{
	return (LOOPACTIONS_T)ui.comboBox_LoopActionsType->itemData(ui.comboBox_LoopActionsType->currentIndex()).toInt();
}

bool SettingsDialog::IsUseLangModel() const
{
	return ui.checkBox_UseLangModel->isChecked();
}

int SettingsDialog::GetNGramSize() const
{
	return ui.spinBox_NGramSize->value();
}




void SettingsDialog::OnAccepted()
{
	settings.beginGroup("kwFinderPlugin");

	settings.setValue("pathToHmm", ui.lineEdit_pathToHmm->text());
	settings.setValue("pathToDict", ui.lineEdit_pathToDict->text());
	settings.setValue("dictInUtf8", ui.checkBox_useUtf8->isChecked());
	settings.setValue("pathToTiedls", ui.lineEdit_pathToTiedls->text());

	settings.setValue("ProcessingDir", ui.lineEdit_processingDir->text());
	settings.setValue("ReportsDir", ui.lineEdit_reportsDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());


	settings.setValue("PruningType", ui.comboBox_PruningType->currentText());
	settings.setValue("PruningPar", ui.doubleSpinBox_PruningPar->value());
	settings.setValue("LoopActionsType", ui.comboBox_LoopActionsType->currentText());
	settings.setValue("UseLangModel", ui.checkBox_UseLangModel->isChecked());
	settings.setValue("NGramSize", ui.spinBox_NGramSize->value());

	settings.setValue("NBestType", ui.comboBox_NBestType->currentText());
	settings.setValue("NBestCount", ui.spinBox_NBestCount->value());
	settings.setValue("NBestCapacity", ui.spinBox_NBestCapacity->value());
	settings.setValue("PrintLatticeOutput", ui.checkBox_PrintLatticeOutput->isChecked());
	settings.setValue("LatticeOutputFile", ui.lineEdit_LatticeOutputFile->text());
	settings.setValue("UseLatticeOutputDir", ui.checkBox_LatticeOutputDir->isChecked());
	settings.setValue("LatticeOutputDir", ui.lineEdit_LatticeOutputDir->text());



	settings.endGroup();
}

void SettingsDialog::OnRejected()
{
	settings.beginGroup("kwFinderPlugin");

	ui.lineEdit_pathToHmm->setText(settings.value("pathToHmm", "kwFinderPluginData/hmmdefs").toString());
	ui.lineEdit_pathToDict->setText(settings.value("pathToDict", "kwFinderPluginData/dictUsed.txt").toString());
	ui.checkBox_useUtf8->setChecked(settings.value("dictInUtf8", false).toBool());
	ui.lineEdit_pathToTiedls->setText(settings.value("pathToTiedls", "kwFinderPluginData/tiedls.txt").toString());
	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());


	int index = ui.comboBox_PruningType->findText(settings.value("PruningType", "NO_PRUN").toString());
	ui.comboBox_PruningType->setCurrentIndex(index == -1 ? 0: index); 
	ui.doubleSpinBox_PruningPar->setValue(settings.value("PruningPar",0.).toDouble());
	index = ui.comboBox_LoopActionsType->findText(settings.value("LoopActionsType", "NO_LOOP").toString());
	ui.comboBox_LoopActionsType->setCurrentIndex(index == -1 ? 0 : index);
	ui.checkBox_UseLangModel->setChecked(settings.value("UseLangModel", false).toBool());
	ui.spinBox_NGramSize->setValue(settings.value("NGramSize", 0).toInt());
	index = ui.comboBox_NBestType->findText(settings.value("NBestType", "NO_NBEST").toString());
	ui.comboBox_NBestType->setCurrentIndex(index == -1? 0 : index);

	ui.spinBox_NBestCount->setValue(settings.value("NBestCount", 0).toInt());
	ui.spinBox_NBestCapacity->setValue(settings.value("NBestCapacity", 0).toInt());
	ui.checkBox_PrintLatticeOutput->setChecked(settings.value("PrintLatticeOutput", false).toBool());
	ui.lineEdit_LatticeOutputFile->setText(settings.value("LatticeOutputFile", "").toString());

	ui.checkBox_LatticeOutputDir->setChecked(settings.value("UseLatticeOutputDir", false).toBool());
	ui.lineEdit_LatticeOutputDir->setText(settings.value("LatticeOutputDir", "").toString());




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

void SettingsDialog::on_toolButton_setLatticeOutputDir_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Установка директории для сохранения латисов"),
						ui.lineEdit_LatticeOutputDir->text());
	if ( !fileName.isEmpty() )
	{
		ui.lineEdit_LatticeOutputDir->setText(fileName);
	}
}
