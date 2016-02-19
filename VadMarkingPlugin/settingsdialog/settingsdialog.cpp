#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);

	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));
	
	Init();
}


void SettingsDialog::UpdateCutModelsList(const QString & models_dir,
							bool use_cut_models,
							const QMap<QString, bool> & selected_state)
{
	ui.groupBox__cut_models->setChecked(use_cut_models);
	QStringList files = QDir(models_dir).entryList(QStringList() << "*.*", QDir::Files);
	
	ui.listWidget_cur_models->clear();
	
	QString name("pause");
	QMap<QString, bool>::const_iterator iter;


	QListWidgetItem * item = new QListWidgetItem(name);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
	iter = selected_state.find(name);
	item->setCheckState(iter != selected_state.end() ? (*iter ? Qt::Checked : Qt::Unchecked ): Qt::Unchecked);
	ui.listWidget_cur_models->addItem(item);


	foreach(const QString & file_path, files)
	{
		name = QFileInfo(file_path).completeBaseName();
		item = new QListWidgetItem(name);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		iter = selected_state.find(name);
		item->setCheckState(iter != selected_state.end() ? (*iter ? Qt::Checked : Qt::Unchecked ): Qt::Unchecked);
		ui.listWidget_cur_models->addItem(item);
	}
}

void SettingsDialog::Init()
{
	settings.beginGroup("VadPlugin");

	ui.checkBox_restartProcessing->setChecked(settings.value("restartProcessing", true).toBool());

	ui.lineEdit_processingDir->setText(settings.value("processingDir", "").toString());
	ui.lineEdit_reportsDir->setText(settings.value("reportDir", "").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.lineEdit_modiffedFilesDir->setText(settings.value("modiffedFilesDir", "").toString());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());
	ui.checkBox_cutNoice->setChecked(settings.value("cutNoice", false).toBool());

	ui.lineEdit_umbModel_dc->setText(settings.value("ubmModel_dc", "vadmarkingplugindata/dcModels/ubm.gmm").toString());
	ui.lineEdit_speechModel_dc->setText(settings.value("speechModel_dc", "vadmarkingplugindata/dcModels/speech.gmm").toString());
	ui.lineEdit_pauseModel_dc->setText(settings.value("pauseModel_dc", "vadmarkingplugindata/dcModels/pause.gmm").toString());
	ui.doubleSpinBox_tresholdParam_dc->setValue(settings.value("tresholdParam_dc", 0.0071).toDouble());
	ui.groupBox_dc->setChecked(settings.value("useDictorsCount", true).toBool());
	ui.spinBox_largeWinSize->setValue(settings.value("largeWinSize", 100000).toInt());

	QString str =
	"{	\n"
	"\"vadModels\":[\n"
			"{\n"
			"\"modelName\":\"vad1\",\n"
			"\"speechModelPath\":\"vadmarkingplugindata/speech40000260.mdl\", \n"
			"\"pauseModelPath\":\"vadmarkingplugindata/pause_mk40000260.mdl\",\n"
			"\"modelsDir\":\"vadmarkingplugindata/noices&voices\",\n"
			"\"modelsMasks\":[],\n"
			"\"useParams2\":true,\n"
			"\"params\":{\"StepWin\": \"0.010\", \"Version\": \"1.0\", \"MinFreq\": 0, \"SampleRate\": 8000, \"WavType\": 1, \"BitsPerSample\": 16, \"Channels\": 1,\n"
			"		\"PreprocessMode\":\"MFCC\",\"NumBankFilters\": 26, \"CepLiftering\": \"22.0\", \"MaxFreq\": 4000, \"SPTHRESHOLD\":\"0\",\n"
			"		\"normalize\" : 0, \"DurWin\": \"0.025\", \"PreEmphCoeff\": \"0.97\", \"DURSORTNOISE\":\"0.4\", \"DURVOICE\":\"0.15\",\n"
			"		\"showNoice\":true, \"showModelNames\":true},\n"
			"\"params2\":{\"StepWin\": \"0.010\", \"Version\": \"1.0\", \"MinFreq\": 0, \"SampleRate\": 8000, \"WavType\": 1, \"BitsPerSample\": 16, \"Channels\": 1,\n"
			"		\"PreprocessMode\":\"MFCC\",\"NumBankFilters\": 26, \"CepLiftering\": \"22.0\", \"MaxFreq\": 4000, \"SPTHRESHOLD\":\"0\",\n"
			"		\"normalize\" : 0, \"DurWin\": \"0.025\", \"PreEmphCoeff\": \"0.97\", \"DURSORTNOISE\":\"0.04\", \"DURVOICE\":\"0.15\",\n"
			"		\"finDurNoise\":0.4, \n"
			"		\"speechList\":[\"unknown_m40000260\", \"unknown_f40000260\", \"unknown_child40000260\", \"__iron_w40000260\"],\n"
			"		\"separatorList\":[\"__glassman40000260\", \"__music40000260\", \"__tone240000260\", \"__zameni_gudok40000260\"]}\n"
			"}	\n"
			"	]\n"
	"}";



	vadPrams = settings.value("vadSettings", str).toString().toUtf8().data();
	try
	{
		json_spirit::mValue v;
		
		if ( json_spirit::read(vadPrams, v) )
		{
			json_spirit::mObject & jsonModel = v.get_obj()["vadModels"].get_array()[0].get_obj();

			ui.lineEdit_speechModel->setText(QString::fromUtf8(jsonModel["speechModelPath"].get_str().data()));
			ui.lineEdit_pauseModel->setText(QString::fromUtf8(jsonModel["pauseModelPath"].get_str().data()));
			ui.lineEdit_noSpeechModel->setText(QString::fromUtf8(jsonModel["modelsDir"].get_str().data()));

			//Грузим состояния моделей.
			QMap<QString,bool> cut_models_state;
			bool use_cut_models = true;

			json_spirit::mObject::iterator iter = jsonModel.find("cut_models");

			if ( iter != jsonModel.end() )
			{
				try
				{
					json_spirit::mObject & obj = iter->second.get_obj();
					use_cut_models = obj["use"].get_bool();

					json_spirit::mArray & cm_arr = obj["models"].get_array();

					for (json_spirit::mArray::iterator iter = cm_arr.begin(), end = cm_arr.end();
						iter != end; iter++ )
					{
						json_spirit::mObject & obj = iter->get_obj();
						cut_models_state[QString::fromUtf8(obj["name"].get_str().c_str())] = obj["use"].get_bool();
					}
				}
				catch(...)
				{
					cut_models_state.clear();
					use_cut_models = true;
				}
			}

			UpdateCutModelsList(ui.lineEdit_noSpeechModel->text(), use_cut_models, cut_models_state);

			//set params:
			json_spirit::mObject & jsonParams = jsonModel["params"].get_obj();

			std::string _str = jsonParams["DurWin"].get_str();
			ui.doubleSpinBox_durWin->setValue(atof(_str.data()));
			//qDebug() << "DurWin:" << ui.doubleSpinBox_durWin->value();

			ui.doubleSpinBox_stepWin->setValue(atof(jsonParams["StepWin"].get_str().data()));
			ui.doubleSpinBox_PreEmphCoeff->setValue(atof(jsonParams["PreEmphCoeff"].get_str().data()));
			ui.spinBox_NumBankFilters->setValue(jsonParams["NumBankFilters"].get_int());
			ui.spinBox_MinFreq->setValue(jsonParams["MinFreq"].get_int());
			ui.spinBox_MaxFreq->setValue(jsonParams["MaxFreq"].get_int());
			ui.doubleSpinBox_CepLiftering->setValue(atof(jsonParams["CepLiftering"].get_str().data()));
			ui.doubleSpinBox_DURSORTNOISE->setValue(atof(jsonParams["DURSORTNOISE"].get_str().data()));
			ui.doubleSpinBox_DURVOICE->setValue(atof(jsonParams["DURVOICE"].get_str().data()));

			ui.checkBox_showNoice->setChecked(jsonParams["showNoice"].get_bool());
			ui.checkBox_showModelNames->setChecked(jsonParams["showModelNames"].get_bool());

			ui.comboBox_PreprocessMode->clear();
			ui.comboBox_PreprocessMode->addItem("MFCC");
			ui.comboBox_PreprocessMode->addItem("PLP");
			ui.comboBox_PreprocessMode->addItem("LPCC");
			int index = ui.comboBox_PreprocessMode->findText(QString::fromUtf8(jsonParams["PreprocessMode"].get_str().data()));
			ui.comboBox_PreprocessMode->setCurrentIndex( index == -1 ? 0 : index );
			ui.spinBox_SPTHRESHOLD->setValue(atoi(jsonParams["SPTHRESHOLD"].get_str().data()));

			//set params2:
			ui.groupBox_params2->setChecked(jsonModel["useParams2"].get_bool());

			json_spirit::mObject & jsonParams2 = jsonModel["params2"].get_obj();

			ui.doubleSpinBox_durWin_2->setValue(atof(jsonParams2["DurWin"].get_str().data()));
			ui.doubleSpinBox_stepWin_2->setValue(atof(jsonParams2["StepWin"].get_str().data()));
			ui.doubleSpinBox_PreEmphCoeff_2->setValue(atof(jsonParams2["PreEmphCoeff"].get_str().data()));
			ui.spinBox_NumBankFilters_2->setValue(jsonParams2["NumBankFilters"].get_int());
			ui.spinBox_MinFreq_2->setValue(jsonParams2["MinFreq"].get_int());
			ui.spinBox_MaxFreq_2->setValue(jsonParams2["MaxFreq"].get_int());
			ui.doubleSpinBox_CepLiftering_2->setValue(atof(jsonParams2["CepLiftering"].get_str().data()));
			ui.doubleSpinBox_DURSORTNOISE_2->setValue(atof(jsonParams2["DURSORTNOISE"].get_str().data()));
			ui.doubleSpinBox_DURVOICE_2->setValue(atof(jsonParams2["DURVOICE"].get_str().data()));

			ui.comboBox_PreprocessMode_2->clear();
			ui.comboBox_PreprocessMode_2->addItem("MFCC");
			ui.comboBox_PreprocessMode_2->addItem("PLP");
			ui.comboBox_PreprocessMode_2->addItem("LPCC");
			index = ui.comboBox_PreprocessMode_2->findText(QString::fromUtf8(jsonParams2["PreprocessMode"].get_str().data()));
			ui.comboBox_PreprocessMode_2->setCurrentIndex( index == -1 ? 0 : index );
			ui.spinBox_SPTHRESHOLD_2->setValue(atoi(jsonParams2["SPTHRESHOLD"].get_str().data()));
			ui.doubleSpinBox_finDurNoise->setValue(jsonParams2["finDurNoise"].get_real());

			ui.listWidget_speechList->clear();
			json_spirit::mArray & ar = jsonParams2["speechList"].get_array();
			
			for ( int i = 0; i < ar.size(); i++ )
			{
				QListWidgetItem * item = new QListWidgetItem(QString::fromUtf8(ar[i].get_str().data()));
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
				ui.listWidget_speechList->addItem(item);
			}

			ui.listWidget_separatorList->clear();
			json_spirit::mArray & ar2 = jsonParams2["separatorList"].get_array();

			for ( int i = 0; i < ar2.size(); i++ )
			{
				QListWidgetItem * item = new QListWidgetItem(QString::fromUtf8(ar2[i].get_str().data()));
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
				ui.listWidget_separatorList->addItem(item);
			}
		}
		else 
			qCritical() << "Cannot read vadParams";
	}
	catch(std::exception & e )
	{
		qCritical() << "parce error: " << QString::fromStdString(e.what());
	}
	settings.endGroup();
}

SettingsDialog::~SettingsDialog()
{
}


json_spirit::mObject SettingsDialog::GetCutModelsDesc() const
{
	//Описание моделей.
	json_spirit::mObject cut_models_desc;
	cut_models_desc["use"] = ui.groupBox__cut_models->isChecked();
	json_spirit::mArray cut_models;

	for (int i = 0; i < ui.listWidget_cur_models->count(); i++ )
	{
		json_spirit::mObject obj;
		obj["name"] = ui.listWidget_cur_models->item(i)->text().toUtf8().data();
		obj["use"] =ui.listWidget_cur_models->item(i)->checkState() == Qt::Checked ? true : false;
		cut_models.push_back(obj);
	}
	cut_models_desc["models"] = cut_models;
	return cut_models_desc;
}

QPair<bool, QMap<QString, bool> > SettingsDialog::GetCutModelsDescInternal() const
{
	QPair<bool, QMap<QString, bool> > res;
	res.first = ui.groupBox__cut_models->isChecked();

	for (int i = 0; i < ui.listWidget_cur_models->count(); i++ )
		res.second[ui.listWidget_cur_models->item(i)->text()] = ui.listWidget_cur_models->item(i)->checkState() == Qt::Checked ? true : false;
	
	return res;
}


void SettingsDialog::OnAccepted()
{

	settings.beginGroup("VadPlugin");


	settings.setValue("restartProcessing", ui.checkBox_restartProcessing->isChecked());


	settings.setValue("processingDir", ui.lineEdit_processingDir->text());
	settings.setValue("reportDir", ui.lineEdit_reportsDir->text());
	settings.setValue("modiffedFilesDir", ui.lineEdit_modiffedFilesDir->text());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("cutNoice", ui.checkBox_cutNoice->isChecked());


	settings.setValue("ubmModel_dc", ui.lineEdit_umbModel_dc->text());
	settings.setValue("speechModel_dc", ui.lineEdit_speechModel_dc->text());
	settings.setValue("pauseModel_dc", ui.lineEdit_pauseModel_dc->text());
	settings.setValue("useDictorsCount", ui.groupBox_dc->isChecked());

	settings.setValue("tresholdParam_dc", ui.doubleSpinBox_tresholdParam_dc->value());
	settings.setValue("largeWinSize", ui.spinBox_largeWinSize->value());


	json_spirit::mObject vadModel;
	vadModel["modelName"] = "vad1";
	vadModel["speechModelPath"] = ui.lineEdit_speechModel->text().toUtf8().data();
	vadModel["pauseModelPath"] = ui.lineEdit_pauseModel->text().toUtf8().data();
	vadModel["modelsDir"] = ui.lineEdit_noSpeechModel->text().toUtf8().data();

	//Описание моделей.
	vadModel["cut_models"] = GetCutModelsDesc();

	vadModel["modelsMasks"] = json_spirit::mArray();
	vadModel["useParams2"] = ui.groupBox_params2->isChecked();



	json_spirit::mObject params;

	params["DurWin"] = QString::number(ui.doubleSpinBox_durWin->value()).toStdString();
	params["StepWin"] = QString::number(ui.doubleSpinBox_stepWin->value()).toStdString();
	params["PreEmphCoeff"] =  QString::number(ui.doubleSpinBox_PreEmphCoeff->value()).toStdString();
	params["NumBankFilters"] = ui.spinBox_NumBankFilters->value();
	params["MinFreq"] = ui.spinBox_MinFreq->value();
	params["MaxFreq"] = ui.spinBox_MaxFreq->value();
	params["CepLiftering"] = QString::number(ui.doubleSpinBox_CepLiftering->value()).toStdString();
	params["showNoice"] =  ui.checkBox_showNoice->isChecked();
	params["showModelNames"] =  ui.checkBox_showModelNames->isChecked();
	params["SPTHRESHOLD"] = QString::number(ui.spinBox_SPTHRESHOLD->value()).toStdString();
	params["DURSORTNOISE"] = QString::number(ui.doubleSpinBox_DURSORTNOISE->value()).toStdString();
	params["DURVOICE"] = QString::number( ui.doubleSpinBox_DURVOICE->value()).toStdString();
	params["PreprocessMode"] = ui.comboBox_PreprocessMode->currentText().toStdString();
	
	params["normalize"] = 0;
	params["Version"] = "1.0";
	params["SampleRate"] = 8000;
	params["WavType"] = 1;
	params["BitsPerSample"] = 16;
	params["Channels"] = 1;


	json_spirit::mObject params2;

	params2["DurWin"] = QString::number(ui.doubleSpinBox_durWin_2->value()).toStdString();
	params2["StepWin"] = QString::number(ui.doubleSpinBox_stepWin_2->value()).toStdString();
	params2["PreEmphCoeff"] =  QString::number(ui.doubleSpinBox_PreEmphCoeff_2->value()).toStdString();
	params2["NumBankFilters"] = ui.spinBox_NumBankFilters_2->value();
	params2["MinFreq"] = ui.spinBox_MinFreq_2->value();
	params2["MaxFreq"] = ui.spinBox_MaxFreq_2->value();
	params2["CepLiftering"] = QString::number(ui.doubleSpinBox_CepLiftering_2->value()).toStdString();

	params2["SPTHRESHOLD"] = QString::number(ui.spinBox_SPTHRESHOLD_2->value()).toStdString();
	params2["DURSORTNOISE"] = QString::number(ui.doubleSpinBox_DURSORTNOISE_2->value()).toStdString();
	params2["DURVOICE"] = QString::number(ui.doubleSpinBox_DURVOICE_2->value()).toStdString();
	params2["finDurNoise"] = ui.doubleSpinBox_finDurNoise->value();
	params2["PreprocessMode"] = ui.comboBox_PreprocessMode_2->currentText().toStdString();
	
	params2["normalize"] = 0;
	params2["Version"] = "1.0";
	params2["SampleRate"] = 8000;
	params2["WavType"] = 1;
	params2["BitsPerSample"] = 16;
	params2["Channels"] = 1;

	json_spirit::mArray ar;
	for ( int i = 0; i < ui.listWidget_speechList->count(); i++ )
		ar.push_back(ui.listWidget_speechList->item(i)->text().toUtf8().data());

	params2["speechList"] = ar;

	json_spirit::mArray ar2;
	for ( int i = 0; i < ui.listWidget_separatorList->count(); i++ )
		ar2.push_back(ui.listWidget_separatorList->item(i)->text().toUtf8().data());
	params2["separatorList"] = ar2;

	vadModel["params"] = params;
	vadModel["params2"] = params2;
	
	json_spirit::mArray vadModelsArr;
	vadModelsArr.push_back(vadModel);
	
	json_spirit::mObject vadModels;
	vadModels["vadModels"] = vadModelsArr;

	vadPrams = json_spirit::write(vadModels, json_spirit::pretty_print | json_spirit::raw_utf8);

	settings.setValue("vadSettings", QString::fromUtf8(vadPrams.data()));

	settings.endGroup();
}


void SettingsDialog::OnRejected()
{
	Init();
}

void SettingsDialog::on_toolButton_setPauseModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл модели паузы"),
												 "",
												 QString::fromUtf8("Модель (*.mdl)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_pauseModel->setText(fileName);
}

void SettingsDialog::on_toolButton_SetSpeechModel_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл модели речи"),
												 "",
												 QString::fromUtf8("Модель (*.mdl)"));
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

void SettingsDialog::on_toolButton_lineEdit_noSpeechModel_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Установить папку с моделями шума"),
												ui.lineEdit_noSpeechModel->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
	{
		ui.lineEdit_noSpeechModel->setText(dir);
		
		QPair<bool, QMap<QString, bool> > state = GetCutModelsDescInternal();
		UpdateCutModelsList(dir,state.first, state.second);
	}
}

void SettingsDialog::on_toolButton_update_cut_model_clicked()
{
	QPair<bool, QMap<QString, bool> > state = GetCutModelsDescInternal();
	UpdateCutModelsList(ui.lineEdit_noSpeechModel->text(),state.first, state.second);
}


void SettingsDialog::on_toolButton_setProcessingDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Установить папку для обработки"),
												ui.lineEdit_processingDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_processingDir->setText(dir);
}

void SettingsDialog::on_toolButton_setReportsDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Установить папку для отчётов"),
												ui.lineEdit_reportsDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_reportsDir->setText(dir);
}

void SettingsDialog::on_toolButton_setModiffedFilesDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Установить папку для модиффицированных файлов"),
												ui.lineEdit_modiffedFilesDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_modiffedFilesDir->setText(dir);
}

void SettingsDialog::on_toolButton_addToSpeechList_clicked()
{
	QListWidgetItem * item = new QListWidgetItem("new model");
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	ui.listWidget_speechList->addItem(item);
}

void SettingsDialog::on_toolButton_delFromSpeechList_clicked()
{
	delete ui.listWidget_speechList->currentItem();
}

void SettingsDialog::on_toolButton_addToSeparatorList_clicked()
{
	QListWidgetItem * item = new QListWidgetItem("new model");
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	ui.listWidget_separatorList->addItem(item);
}

void SettingsDialog::on_toolButton_delFromSeparatorList_clicked()
{
	delete ui.listWidget_separatorList->currentItem();
}


void SettingsDialog::on_toolButton_ubmModel_dc_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл модели ubm"),
												 "",
												 QString::fromUtf8("Модель (*.mdl)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_umbModel_dc->setText(fileName);
}

void SettingsDialog::on_toolButton_speechModel_dc_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл модели речи (кол. дикт.)"),
												 "",
												 QString::fromUtf8("Модель (*.mdl)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_speechModel_dc->setText(fileName);
}

void SettingsDialog::on_toolButton_pauseModel_dc_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл модели паузы (кол. дикт.)"),
												 "",
												 QString::fromUtf8("Модель (*.mdl)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_pauseModel_dc->setText(fileName);
}