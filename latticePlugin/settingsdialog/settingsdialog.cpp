#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>


SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);
	settings.beginGroup("latticePlugin");

	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());

	ui.lineEdit_programmFileName->setText(settings.value("ProgrammFileName", "latticePluginData/latticeSTD_qt.exe").toString());
	ui.checkBox_showBest->setChecked(settings.value("showBest", false).toBool());
	ui.spinBox_Start_Threshold->setValue(settings.value("Start_Threshold", 3000000).toInt());
	ui.spinBox_End_Treshold->setValue(settings.value("End_Treshold", 5000000).toInt());
	ui.lineEdit_latticeFile->setText(settings.value("latticeFile", "lattice.lat").toString());
	ui.checkBox_usePostProcessing->setChecked(settings.value("usePostProcessing", false).toBool());


	//! Загрузим ключи.
	settings.beginGroup("keys");
	QStringList keys = settings.childKeys();

	//! Добавим параметры.
	QMap<QString,QString> map;

	ui.tableWidget_parameters->clearContents();

	//! Ключи по умолчанию.
	if ( !keys.size() )
	{
		map["--trace"]					="0";
		map["--MLF-file"]				="out.lab";
		map["--dictionary-file"]		="kw_vf_ph_tr.txt";
		map["--score-method"]			="LogLikeliBaumWelsh";
		map["--filter-method"]			="BestTimeBestScore";
		map["--primary-tip"]			="1.00000";
		map["--primary-acsf"]			="1.00000";
		map["--primary-lmsf"]			="1.00000";
		map["--primary-kwsf"]			="0.00001";

		for (QMap<QString,QString>::iterator iter = map.begin(); 
				iter != map.end(); iter++ )
		{
			//! Добавим элементы.
			QTableWidgetItem * item = new QTableWidgetItem(iter.key());
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			ui.tableWidget_parameters->insertRow(ui.tableWidget_parameters->rowCount());
			ui.tableWidget_parameters->setItem(ui.tableWidget_parameters->rowCount()-1, 0, item);

			item = new QTableWidgetItem(*iter);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			ui.tableWidget_parameters->setItem(ui.tableWidget_parameters->rowCount()-1, 1, item);
		}
	}
	else 
	{
		for ( QStringList::iterator iter = keys.begin();
			 iter != keys.end(); iter++ )
		{
			QTableWidgetItem * item = new QTableWidgetItem(*iter);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			ui.tableWidget_parameters->insertRow(ui.tableWidget_parameters->rowCount());
			ui.tableWidget_parameters->setItem(ui.tableWidget_parameters->rowCount()-1, 0, item);

			item = new QTableWidgetItem(settings.value(*iter).toString());
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			ui.tableWidget_parameters->setItem(ui.tableWidget_parameters->rowCount()-1, 1, item);			
		}
	}
	ui.tableWidget_parameters->resizeColumnsToContents();

	settings.endGroup();

	settings.endGroup();

	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::OnAccepted()
{
	settings.beginGroup("latticePlugin");

	settings.setValue("ProcessingDir", ui.lineEdit_processingDir->text());
	settings.setValue("ReportsDir", ui.lineEdit_reportsDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());
	settings.setValue("ProgrammFileName", ui.lineEdit_programmFileName->text());

	settings.setValue("Start_Threshold", ui.spinBox_Start_Threshold->value());
	settings.setValue("End_Treshold", ui.spinBox_End_Treshold->value());
	settings.setValue("showBest", ui.checkBox_showBest->isChecked());
	settings.setValue("usePostProcessing", ui.checkBox_usePostProcessing->isChecked());
	settings.setValue("latticeFile", ui.lineEdit_latticeFile->text());

	//! Загрузим ключи.
	settings.beginGroup("keys");
	settings.remove("");

	for ( int i = 0; i < ui.tableWidget_parameters->rowCount(); i++ )
	{
		QString key = ui.tableWidget_parameters->item(i, 0)->text();
		if ( !key.isEmpty() )
		{
			QString value = ui.tableWidget_parameters->item(i, 1)->text();
			settings.setValue(key, value);
		}
	}

	settings.endGroup();

	settings.endGroup();
}

QMap<QString, QString> SettingsDialog::GetParameters() const
{
	QMap<QString, QString> map;
	for ( int i = 0; i < ui.tableWidget_parameters->rowCount(); i++ )
		map[ui.tableWidget_parameters->item(i, 0)->text()] = ui.tableWidget_parameters->item(i, 1)->text();
	return map;
}


void SettingsDialog::OnRejected()
{
	settings.beginGroup("latticePlugin");

	ui.lineEdit_processingDir->setText(settings.value("ProcessingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("ReportsDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());
	ui.lineEdit_programmFileName->setText(settings.value("ProgrammFileName", "latticePluginData/latticeSTD_qt.exe").toString());
	
	ui.checkBox_showBest->setChecked(settings.value("showBest", false).toBool());
	ui.spinBox_Start_Threshold->setValue(settings.value("Start_Threshold", 3000000).toInt());
	ui.spinBox_End_Treshold->setValue(settings.value("End_Treshold", 5000000).toInt());
	ui.lineEdit_latticeFile->setText(settings.value("latticeFile", "lattice.lat").toString());
	ui.checkBox_usePostProcessing->setChecked(settings.value("usePostProcessing", false).toBool());



	//! Загрузим ключи.
	settings.beginGroup("keys");
	QStringList keys = settings.childKeys();

	for ( int i = 0; i < ui.tableWidget_parameters->rowCount(); i++ )
	{
		QString key = ui.tableWidget_parameters->item(i, 0)->text();
		QString value = ui.tableWidget_parameters->item(i, 1)->text();

		if ( keys.indexOf(key) != -1 )
			value = settings.value(key).toString();
		ui.tableWidget_parameters->item(i, 1)->setText(value);
	}

	settings.endGroup();


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

void SettingsDialog::on_pushButton_addParam_clicked()
{
	QTableWidgetItem * item = new QTableWidgetItem("");
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	ui.tableWidget_parameters->insertRow(ui.tableWidget_parameters->rowCount());
	ui.tableWidget_parameters->setItem(ui.tableWidget_parameters->rowCount()-1, 0, item);

	item = new QTableWidgetItem("");
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	ui.tableWidget_parameters->setItem(ui.tableWidget_parameters->rowCount()-1, 1, item);
}

void SettingsDialog::on_pushButton_delParam_clicked()
{
	int index = ui.tableWidget_parameters->currentRow();

	if ( index )
		ui.tableWidget_parameters->removeRow(index);
}