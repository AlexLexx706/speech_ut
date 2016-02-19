#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QVariant>
#include <QList>
#include "../json_spirit/json_spirit.h"


SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);
	connect(this, SIGNAL(accepted()),this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()),this, SLOT(OnRejected()));

	ui.comboBox_PruningType->clear();
	ui.comboBox_PruningType->addItem("NO_PRUN", (int)NO_PRUN);
	ui.comboBox_PruningType->addItem("PATHNUM", (int)PATHNUM);
	ui.comboBox_PruningType->addItem("PATHPROB", (int)PATHPROB);
	ui.comboBox_PruningType->addItem("PATHPRATIO", (int)PATHPRATIO);

	ui.comboBox_LoopActionsType->clear();
	ui.comboBox_LoopActionsType->addItem("NO_LOOP", (int)NO_LOOP);
	ui.comboBox_LoopActionsType->addItem("LOOP_RECORD", (int)LOOP_RECORD);
	ui.comboBox_LoopActionsType->addItem("LOOP_NO_RECORD", (int)LOOP_NO_RECORD);

	ui.comboBox_NBestType->clear();
	ui.comboBox_NBestType->addItem("NO_NBEST", (int)NO_NBEST);
	ui.comboBox_NBestType->addItem("LATTICE", (int)LATTICE);
	ui.comboBox_NBestType->addItem("WORD_DEPENDENT", (int)WORD_DEPENDENT);


	ui.comboBox_features_type->clear();
	ui.comboBox_features_type->addItem("MFCC", (int)MFCC);
	ui.comboBox_features_type->addItem("PLP", (int)PLP);
	ui.comboBox_features_type->addItem("LPCC", (int)LPCC);
	ui.comboBox_features_type->addItem("MFCC_FB", (int)MFCC_FB);
	ui.comboBox_features_type->addItem("PLP_VTLN", (int)PLP_VTLN);
	ui.comboBox_features_type->addItem("PLP_RASTA", (int)PLP_RASTA);


	ui.comboBox_normalize_type->clear();
	ui.comboBox_normalize_type->addItem("NO_NORM", (int)NO_NORM);
	ui.comboBox_normalize_type->addItem("ZEROMEAN", (int)ZEROMEAN);

	ui.comboBox_delta_type->clear();
	ui.comboBox_delta_type->addItem("NO_DELTA", (int)NO_DELTA);
	ui.comboBox_delta_type->addItem("DELTA", (int)DELTA);
	ui.comboBox_delta_type->addItem("DELTADELTA", (int)DELTADELTA);
	ui.comboBox_delta_type->addItem("SHIFTED_DELTA", (int)SHIFTED_DELTA);
	ui.comboBox_delta_type->addItem("DELTA3", (int)DELTA3);

	ui.listWidget_setup_params->clear();
	for(int i = 0; i < 11; i++ )
	{
		QListWidgetItem * item = new QListWidgetItem("0.0");
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui.listWidget_setup_params->addItem(item);
	}

	OnRejected();
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
	settings.beginGroup("VerbatimPlugin");

	settings.setValue("ModelFileName", ui.lineEdit_modelFileName->text());
	settings.setValue("PruningType", ui.comboBox_PruningType->currentText());
	settings.setValue("PruningPar", ui.doubleSpinBox_PruningPar->value());
	settings.setValue("LoopActionsType", ui.comboBox_LoopActionsType->currentText());
	settings.setValue("UseLangModel", ui.checkBox_UseLangModel->isChecked());
	settings.setValue("NGramSize", ui.spinBox_NGramSize->value());

	settings.setValue("ProcessingDir", ui.lineEdit_processingDir->text());
	settings.setValue("ReportsDir", ui.lineEdit_reportsDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());


	settings.setValue("NBestType", ui.comboBox_NBestType->currentText());
	settings.setValue("NBestCount", ui.spinBox_NBestCount->value());
	settings.setValue("NBestCapacity", ui.spinBox_NBestCapacity->value());
	settings.setValue("PrintLatticeOutput", ui.checkBox_PrintLatticeOutput->isChecked());
	settings.setValue("LatticeOutputFile", ui.lineEdit_LatticeOutputFile->text());
	settings.setValue("UseLatticeOutputDir", ui.checkBox_LatticeOutputDir->isChecked());
	settings.setValue("LatticeOutputDir", ui.lineEdit_LatticeOutputDir->text());


	settings.setValue("features_type", ui.comboBox_features_type->itemData(ui.comboBox_features_type->currentIndex()).toInt());
	settings.setValue("normalize_type", ui.comboBox_normalize_type->itemData(ui.comboBox_normalize_type->currentIndex()).toInt());
	settings.setValue("delta_type", ui.comboBox_delta_type->itemData(ui.comboBox_delta_type->currentIndex()).toInt());

	settings.setValue("vtln_struct", ui.groupBox_vtln_struct->isChecked());

	settings.setValue("alpha", ui.lineEdit_alpha->text());
	settings.setValue("f_l", ui.lineEdit_f_l->text());
	settings.setValue("f_h", ui.lineEdit_f_h->text());
	settings.setValue("f_max", ui.lineEdit_f_max->text());
	settings.setValue("f_min", ui.lineEdit_f_min->text());

	settings.setValue("setup_params", ui.groupBox_setup_params->isChecked());
	
	QList<QVariant> sp;
	for(int i = 0; i < ui.listWidget_setup_params->count(); i++ )
		sp << ui.listWidget_setup_params->item(i)->text();
	settings.setValue("setup_params_data", sp);
	settings.endGroup();
}

std::string SettingsDialog::get_plp_json_string_settings() const
{
	json_spirit::mObject obj;
	obj["features_type"] = ui.comboBox_features_type->itemData(ui.comboBox_features_type->currentIndex()).toInt();
	obj["normalize_type"] = ui.comboBox_normalize_type->itemData(ui.comboBox_normalize_type->currentIndex()).toInt();
	obj["delta_type"] = ui.comboBox_delta_type->itemData(ui.comboBox_delta_type->currentIndex()).toInt();

	if ( ui.groupBox_vtln_struct->isChecked() )
	{
		json_spirit::mObject obj_vtln;
		obj_vtln["alpha"] = ui.lineEdit_alpha->text().toDouble();
		obj_vtln["f_l"] = ui.lineEdit_f_l->text().toDouble();
		obj_vtln["f_h"] = ui.lineEdit_f_h->text().toDouble();
		obj_vtln["f_max"] = ui.lineEdit_f_max->text().toDouble();
		obj_vtln["f_min"] = ui.lineEdit_f_min->text().toDouble();
		obj["vtln_struct"] = obj_vtln;
	}

	if ( ui.groupBox_setup_params->isChecked() )
	{
		json_spirit::mArray arr;
		for(int i = 0; i < ui.listWidget_setup_params->count(); i++ )
			arr.push_back( ui.listWidget_setup_params->item(i)->text().toDouble());
		
		obj["setup_params"] = arr;
	}
	return json_spirit::write(obj);
}


void SettingsDialog::OnRejected()
{
	settings.beginGroup("VerbatimPlugin");

	ui.lineEdit_modelFileName->setText(settings.value("ModelFileName", "ViterbiPluginData/hmmdefs+spau").toString());
	int index = ui.comboBox_PruningType->findText(settings.value("PruningType", "NO_PRUN").toString());
	ui.comboBox_PruningType->setCurrentIndex(index == -1 ? 0: index); 
	ui.doubleSpinBox_PruningPar->setValue(settings.value("PruningPar",0.).toDouble());
	index = ui.comboBox_LoopActionsType->findText(settings.value("LoopActionsType", "NO_LOOP").toString());
	ui.comboBox_LoopActionsType->setCurrentIndex(index == -1 ? 0 : index);
	ui.checkBox_UseLangModel->setChecked(settings.value("UseLangModel", false).toBool());
	ui.spinBox_NGramSize->setValue(settings.value("NGramSize", 0).toInt());
	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	index = ui.comboBox_NBestType->findText(settings.value("NBestType", "NO_NBEST").toString());
	ui.comboBox_NBestType->setCurrentIndex(index == -1? 0 : index);

	ui.spinBox_NBestCount->setValue(settings.value("NBestCount", 0).toInt());
	ui.spinBox_NBestCapacity->setValue(settings.value("NBestCapacity", 0).toInt());
	ui.checkBox_PrintLatticeOutput->setChecked(settings.value("PrintLatticeOutput", false).toBool());
	ui.lineEdit_LatticeOutputFile->setText(settings.value("LatticeOutputFile", "").toString());

	ui.checkBox_LatticeOutputDir->setChecked(settings.value("UseLatticeOutputDir", false).toBool());
	ui.lineEdit_LatticeOutputDir->setText(settings.value("LatticeOutputDir", "").toString());


	ui.comboBox_features_type->setCurrentIndex(ui.comboBox_features_type->findData(settings.value("features_type", (int)PLP).toInt()));
	ui.comboBox_normalize_type->setCurrentIndex(ui.comboBox_normalize_type->findData(settings.value("normalize_type", (int)ZEROMEAN).toInt()));
	ui.comboBox_delta_type->setCurrentIndex(ui.comboBox_delta_type->findData(settings.value("delta_type", (int)DELTADELTA).toInt()));

	ui.groupBox_vtln_struct->setChecked(settings.value("vtln_struct", false).toBool());

	ui.lineEdit_alpha->setText(settings.value("alpha", "0").toString());
	ui.lineEdit_f_l->setText(settings.value("f_l", "0").toString());
	ui.lineEdit_f_h->setText(settings.value("f_h", "0").toString());
	ui.lineEdit_f_max->setText(settings.value("f_max", "0").toString());
	ui.lineEdit_f_min->setText(settings.value("f_min", "0").toString());

	ui.groupBox_setup_params->setChecked(settings.value("setup_params", false).toBool());
	QList<QVariant> sp = settings.value("setup_params_data", QList<QVariant>() << "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0"<< "0.0").toList();
	for(int i = 0; i < ui.listWidget_setup_params->count(); i++ )
		ui.listWidget_setup_params->item(i)->setText(sp[i].toString());

	settings.endGroup();
}

void SettingsDialog::on_toolButton_setModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Установка файла модели в фрмате HTK"),
												 ui.lineEdit_modelFileName->text());
	if ( !fileName.isEmpty() )
	{
		ui.lineEdit_modelFileName->setText(fileName);
	}
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