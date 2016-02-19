#include "settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include "../../json_spirit/json_spirit.h"
#include "Language Identification/Recognition.h"
#include <QDir>


bool SettingsDialog::LoadNGrammModels(const QString & path, 
										const QString & mask,
										const QString & mask_aux,
										QMap<QString, QStringList> & out,
										QString & errorDesc,
										int clientID,
										bool loadModelInRecognition,
										bool txtModel)
{
	QDir dirNames(path);
	QStringList names = dirNames.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	
	try
	{
		if ( !names.size() )
			throw QString::fromUtf8("Ошибка: директория:\"%1\" языков пуста").arg(dirNames.path());

		out.clear();

		foreach(const QString & name, names)
		{
			QDir dirChannels(dirNames.path() + "/" + name);
			QStringList channels = dirChannels.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
			
			if ( !channels.size() )
				throw QString::fromUtf8("Ошибка, нет каналов языка:\"%1\", в папке:\"%2\"").arg(name).arg(dirChannels.path());

			foreach(const QString & channel, channels)
			{
				QDir dirFiles(dirChannels.path() + "/" + channel);
				QStringList files;
				
				if ( mask.isEmpty() )
				{
					QStringList __files = dirFiles.entryList(QDir::Files);
					foreach(const QString & name, __files)
						if (QFileInfo(name).suffix().isEmpty())
							files << name;
				}
				else
					files = dirFiles.entryList(QStringList() << mask, QDir::Files);

				QStringList files_aux;

				if ( mask_aux.isEmpty() )
				{
					QStringList __files = dirFiles.entryList(QDir::Files);
					foreach(const QString & name, __files)
						if (QFileInfo(name).suffix().isEmpty())
							files_aux << name;
				}
				else
					files_aux = dirFiles.entryList(QStringList() << mask_aux, QDir::Files);
				
				if ( !files.size() || files.size() != files_aux.size() )

					throw QString::fromUtf8("Отсутствуют файлы, канал:\"%2\" язык:\"%3\"").
							arg(channel).
							arg(name);
				
				out[channel] << name;

				if ( loadModelInRecognition )
				{
					foreach(const QString & recName, files)
					{
						QString filePath = dirFiles.path() + "/" + recName;
						QString filePathAux = QString("%1/%2%3").
												arg(dirFiles.path()).
												arg(QFileInfo(recName).baseName()).
												arg(mask_aux.mid(1));

						
						if( Recognition::LoadNGrammModels(clientID,
															name.toLocal8Bit().data(),
															channel.toLocal8Bit().data(),
															QFileInfo(recName).baseName().toLocal8Bit().data(),
															true,
															txtModel,
															filePath.toLocal8Bit().data(),
															filePath.toLocal8Bit().size(),
															filePathAux.toLocal8Bit().data(),
															filePathAux.toLocal8Bit().size()) != 1 )
						{
							throw QString::fromUtf8("Ошибка: не удаётся загрузить модель, файл:\"%1\"").arg(filePath);						
						}
					}
				}
			}
		}
		return true;
	}
	catch( QString & message )
	{
		errorDesc = message;
	}
	return false;
}

bool SettingsDialog::LoadPhonemsModels(bool noChannels,
									   const QString & path,
									   const QString & mask,
									   QMap<QString, QStringList> & out,
									   QString & errorDesc,
									   int clientID,
										bool loadModelInRecognition)
{
	out.clear();

	if ( !noChannels )
	{
		QDir dirPhonems(path);
		QStringList channels = dirPhonems.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		try
		{
			if ( !channels.size() )
				throw QString::fromUtf8("Ошибка: директория:\"%1\" фонем пуста").arg(dirPhonems.path());

			foreach(const QString & channel, channels)
			{
				QDir dirFiles(dirPhonems.path() + "/" + channel);
				QStringList models;
				
				if ( mask.isEmpty() )
				{
					QStringList _models = dirFiles.entryList(QDir::Files | QDir::Readable);
					foreach(const QString & model, _models)
						if ( QFileInfo(model).suffix().isEmpty() )
							models << model;
				}
				else 
					models = dirFiles.entryList(QStringList() << mask, QDir::Files | QDir::Readable);
				
				if ( !models.size() )
					throw QString::fromUtf8("Ошибка: в директория:\"%1\" канала :\"%2\" пуста.").
						arg(dirFiles.path()).
						arg(channel);

				QStringList & outModels = out[channel];

				foreach(const QString & model, models)
				{
					outModels  << model;

					if (loadModelInRecognition)
					{
						QFile file(dirFiles.path() + "/" + model);
						if ( file.open(QIODevice::ReadOnly) )
						{
							QByteArray data = file.readAll();
							file.close();

							if ( Recognition::LoadPhonemeModels(clientID,
								channel.toLocal8Bit().data(),
								model.toLocal8Bit().data(),
								data.data(),
								data.size()) != 1 )
							{
								throw QString::fromUtf8("Recognition::LoadPhonemeModels error");
							}
						}
						else
							throw QString::fromUtf8("Ошибка немогу открыть файл:\"%1\" канал:\"%2\""
									" модель:\"%3\"").arg(file.fileName()).arg(channel).arg(model);
					}
				}
			}
			return true;
		}
		catch( QString & message )
		{
			errorDesc = message;
		}
	}
	else
	{
		QDir dirModelDirs(path);
		QStringList modelDirs = dirModelDirs.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		try
		{
			if ( !modelDirs.size() )
				throw QString::fromUtf8("Ошибка: директория моделей пуста:\"%1\" фонем пуста").
					arg(dirModelDirs.path());
			QString channel("unknown_channel");
			QString modelName;


			foreach(const QString & modelDir, modelDirs)
			{
				QDir dirFiles(dirModelDirs.path() + "/" + modelDir);
				QStringList models;
				
				if ( mask.isEmpty() )
				{
					QStringList _models = dirFiles.entryList(QDir::Files | QDir::Readable);
					foreach(const QString & model, _models)
						if ( QFileInfo(model).suffix().isEmpty() )
							models << model;
				}
				else 
					models = dirFiles.entryList(QStringList() << mask, QDir::Files | QDir::Readable);
				
				if ( !models.size() )
					throw QString::fromUtf8("Ошибка: директория модели пуста: \"%1\"").arg(dirFiles.path());
				
				modelName = QFileInfo(models[0]).baseName();
				out[channel] << modelName;

				if (loadModelInRecognition)
				{
					QFile file(dirFiles.path() + "/" + models[0]);

					if ( file.open(QIODevice::ReadOnly) )
					{
						QByteArray data = file.readAll();
						file.close();

						if ( Recognition::LoadPhonemeModels(clientID,
							channel.toStdString().c_str(),
							modelName.toStdString().c_str(),
							data.data(),
							data.size()) != 1 )
						{
							throw QString::fromUtf8("Recognition::LoadPhonemeModels error");
						}
					}
					else
						throw QString::fromUtf8("Ошибка немогу открыть файл:\"%1\" канал:\"%2\""
								" модель:\"%3\"").arg(file.fileName()).
										arg(channel).
										arg(modelName);
				}
			}
			return true;
		}
		catch( QString & message )
		{
			errorDesc = message;
		}
	}

	return false;
}

SettingsDialog::SettingsDialog(QSettings & _settings, QWidget *parent)
	: QDialog(parent),
	settings(_settings)
{
	ui.setupUi(this);
	ui.comboBox_ngrammModelType->clear();
	ui.comboBox_ngrammModelType->addItem(QString::fromUtf8("Текстовая модель"), 0);
	ui.comboBox_ngrammModelType->addItem(QString::fromUtf8("Бинарная модель"), 1);

	ui.comboBox_taskType->clear();
	ui.comboBox_taskType->addItem(QString::fromUtf8("Открытая задача"), 0);
	ui.comboBox_taskType->addItem(QString::fromUtf8("Закрытая задача"), 1);


	ui.treeWidget_nGrammModels->UseMultiSelections(true);
	ui.treeWidget_phonemModels->UseMultiSelections(true);
	OnRejected();
	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()), this, SLOT(OnRejected()));
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::OnAccepted()
{
	settings.beginGroup("LanguageIdentificationPlugin");

	settings.setValue("noPhonemChannels", ui.checkBox_noPhonemChannels->isChecked());

	settings.setValue("useMoreServer", ui.groupBox_moreServer->isChecked());
	settings.setValue("moreServerHost", ui.lineEdit_moreServerHost->text());
	settings.setValue("moreServerPort", ui.spinBox_moreServerPort->value());

	settings.setValue("pauseModelPath", ui.lineEdit_pauseModelPath->text());
	settings.setValue("speechModelPath", ui.lineEdit_speechModelPath->text());
	settings.setValue("dictorsModelsDir", ui.lineEdit_dictorsModelsDir->text());
	settings.setValue("dictorsModelsMask", ui.lineEdit_modelsMask->text());

	settings.setValue("restartProcessing", ui.checkBox_restartProcessing->isChecked());

	settings.setValue("saveRecognitionReport", ui.checkBox_saveRecognitionReport->isChecked());
	settings.setValue("sortFilesByLangs", ui.checkBox_sortFilesByLangs->isChecked());
	settings.setValue("createLabFile", ui.checkBox_createLabFile->isChecked());

	
	settings.setValue("nGrammModelsDir", ui.lineEdit_nGrammModelsDir->text());
	settings.setValue("nGrammModelsMasks", ui.lineEdit_nGrammModelsMask->text());
	settings.setValue("nGrammModelsMasks_aux", ui.lineEdit_nGrammModelsMask_aux->text());


	settings.setValue("phonemsModelsDir", ui.lineEdit_phonemsModelsDir->text());
	settings.setValue("phonemsModelsMask", ui.lineEdit_phonemsModelsMask->text());

	QPair<QString, QStringList> curNgramms = ui.treeWidget_nGrammModels->GetCurModels();
	settings.setValue("channelNGrammModels", curNgramms.first);

	QVariantList vl;
	foreach(const QString & m, curNgramms.second)
		vl << m;
	settings.setValue("nGrammModels", QVariant(vl));


	QPair<QString, QStringList> curPhonems = ui.treeWidget_phonemModels->GetCurModels();
	settings.setValue("channelPhonemModels", curPhonems.first);

	vl.clear();
	foreach(const QString & m, curPhonems.second)
		vl << m;
	settings.setValue("phonemsModels", QVariant(vl));

	settings.setValue("wavDir", ui.lineEdit_wavDir->text());
	settings.setValue("outDir",ui.lineEdit_outDir->text());
	settings.setValue("maxBPThreadCount", ui.spinBox_maxBPThreadCount->value());
	settings.setValue("useExternalProcessing", ui.checkBox_useExternalProcessing->isChecked());

	settings.setValue("useVad", ui.groupBox_vad->isChecked());
	settings.setValue("ngrammModelType", ui.comboBox_ngrammModelType->itemData(
		ui.comboBox_ngrammModelType->currentIndex()).toInt());
	settings.setValue("ngrammCount", ui.spinBox_ngrammCount->value());
	settings.setValue("threshold", ui.doubleSpinBox_threshold->value());

	settings.setValue("taskType", ui.comboBox_taskType->itemData(ui.comboBox_taskType->currentIndex()).toInt());

	settings.endGroup();
}

 
void SettingsDialog::OnRejected()
{
	settings.beginGroup("LanguageIdentificationPlugin");

	ui.checkBox_noPhonemChannels->setChecked(settings.value("noPhonemChannels", false).toBool());

	ui.groupBox_moreServer->setChecked(settings.value("useMoreServer", false).toBool());
	ui.lineEdit_moreServerHost->setText(settings.value("moreServerHost", "127.0.0.1").toString());
	ui.spinBox_moreServerPort->setValue(settings.value("moreServerPort", 8030).toInt());

	ui.lineEdit_pauseModelPath->setText(settings.value("pauseModelPath",
			"LanguageIdentificationPlugin\\models\\VADmodels\\_pause.mdl").toString());

	ui.lineEdit_speechModelPath->setText(settings.value("speechModelPath",
			"LanguageIdentificationPlugin\\models\\VADmodels\\_speech.mdl").toString());

	ui.lineEdit_dictorsModelsDir->setText(settings.value("dictorsModelsDir",
			"LanguageIdentificationPlugin\\models\\VADmodels").toString());

	ui.lineEdit_modelsMask->setText(settings.value("dictorsModelsMask", "*.dat").toString());
	ui.checkBox_restartProcessing->setChecked(settings.value("restartProcessing", true).toBool());

	ui.checkBox_saveRecognitionReport->setChecked(settings.value("saveRecognitionReport", true).toBool());
	ui.checkBox_sortFilesByLangs->setChecked(settings.value("sortFilesByLangs", true).toBool());
	ui.checkBox_createLabFile->setChecked(settings.value("createLabFile", true).toBool());


	ui.lineEdit_nGrammModelsDir->setText(settings.value("nGrammModelsDir",
		"LanguageIdentificationPlugin\\models\\ngramms").toString());
	ui.lineEdit_nGrammModelsMask->setText(settings.value("nGrammModelsMasks", "*.mdl").toString());
	ui.lineEdit_nGrammModelsMask_aux->setText(settings.value("nGrammModelsMasks_aux", "*.dev").toString());

	ui.lineEdit_phonemsModelsDir->setText(settings.value("phonemsModelsDir",
		"LanguageIdentificationPlugin\\models\\phoneme").toString());
	ui.lineEdit_phonemsModelsMask->setText(settings.value("phonemsModelsMask", "*").toString());

	QMap<QString, QStringList> curNgramms;
	QString errorDesc;

	if ( !LoadNGrammModels(ui.lineEdit_nGrammModelsDir->text(),
							ui.lineEdit_nGrammModelsMask->text(),
							ui.lineEdit_nGrammModelsMask_aux->text(),
							curNgramms,
							errorDesc) )
	{
		ui.frame_nrammsMessage->show();
		ui.lineEdit_messageNgramm->setText(errorDesc);
	}
	else
		ui.frame_nrammsMessage->hide();

	ui.treeWidget_nGrammModels->Init(curNgramms);
	ui.treeWidget_nGrammModels->SetModels(settings.value("channelNGrammModels", "acoustic").toString(),
											settings.value("nGrammModels", 
												QVariantList() << "eng" << "lit" << "nrusu" << "pol" << "rus").toList());



	QMap<QString, QStringList> curPhonems;

	if( !LoadPhonemsModels(ui.checkBox_noPhonemChannels->isChecked(),
							ui.lineEdit_phonemsModelsDir->text(),
							ui.lineEdit_phonemsModelsMask->text(),
							curPhonems,
							errorDesc) )
	{
		ui.frame_phonemsMessage->show();
		ui.lineEdit_messagePhonems->setText(errorDesc);
	}
	else
		ui.frame_phonemsMessage->hide();

	ui.treeWidget_phonemModels->Init(curPhonems);
	ui.treeWidget_phonemModels->SetModels(settings.value("channelPhonemModels", "acoustic").toString(),
											settings.value("phonemsModels",
												QVariantList() << "arb" << "cmn" << "deu" << "eng").toList());

	ui.lineEdit_wavDir->setText(settings.value("wavDir", "LanguageIdentificationPlugin\\models\\test").toString());
	ui.lineEdit_outDir->setText(settings.value("outDir", "recognition_reports").toString());
	ui.spinBox_maxBPThreadCount->setValue(settings.value("maxBPThreadCount", 1).toInt());
	ui.checkBox_useExternalProcessing->setChecked(settings.value("useExternalProcessing", false).toBool());
	ui.groupBox_vad->setChecked(settings.value("useVad", true).toBool());

	ui.comboBox_ngrammModelType->setCurrentIndex(ui.comboBox_ngrammModelType->findData(
		settings.value("ngrammModelType", 0).toInt()));

	ui.spinBox_ngrammCount->setValue(settings.value("ngrammCount", 3).toInt());
	ui.doubleSpinBox_threshold->setValue(settings.value("threshold", 0).toDouble());
	

	ui.comboBox_taskType->setCurrentIndex(ui.comboBox_taskType->findData(settings.value("taskType", 0).toInt()));
	settings.endGroup();
}

void SettingsDialog::on_toolButton_setPauseModelPath_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл модели паузы"),
												 ui.lineEdit_pauseModelPath->text(),
												 QString::fromLocal8Bit("Модель (*.*)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_pauseModelPath->setText(fileName);

}

void SettingsDialog::on_toolButton_setSpeechModelPath_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Открыть файл модели речи"),
												 ui.lineEdit_speechModelPath->text(),
												 QString::fromLocal8Bit("Модель (*.*)"));
	if ( !fileName.isEmpty() )
		ui.lineEdit_speechModelPath->setText(fileName);
}

void SettingsDialog::on_toolButton_setDictorsModelsDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с моделями вада"),
												ui.lineEdit_dictorsModelsDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
		ui.lineEdit_dictorsModelsDir->setText(dir);
}

void SettingsDialog::on_toolButton_checkNgarmms_clicked()
{
	QMap<QString, QStringList> models;
	QString errorDesc;
	
	if ( LoadNGrammModels(ui.lineEdit_nGrammModelsDir->text(),
							ui.lineEdit_nGrammModelsMask->text(),
							ui.lineEdit_nGrammModelsMask_aux->text(),
							models,
							errorDesc) )
	{
		ui.treeWidget_nGrammModels->Init(models);
		ui.frame_nrammsMessage->hide();
	}
	else
	{
		ui.treeWidget_nGrammModels->Init(QMap<QString, QStringList>());
		ui.frame_nrammsMessage->show();
		ui.lineEdit_messageNgramm->setText(errorDesc);
	}
}

void SettingsDialog::on_toolButton_checkPhonems_clicked()
{
	QMap<QString, QStringList> models;
	QString errorDesc;
	
	if ( LoadPhonemsModels(ui.checkBox_noPhonemChannels->isChecked(),
							ui.lineEdit_phonemsModelsDir->text(),
							ui.lineEdit_phonemsModelsMask->text(),
							models,
							errorDesc) )
	{
		ui.treeWidget_phonemModels->Init(models);
		ui.frame_phonemsMessage->hide();
	}
	else
	{
		ui.treeWidget_phonemModels->Init(QMap<QString, QStringList>());
		ui.frame_phonemsMessage->show();
		ui.lineEdit_messagePhonems->setText(errorDesc);
	}
}

void SettingsDialog::on_checkBox_useExternalProcessing_clicked(bool checked)
{
	if ( !checked )
		ui.spinBox_maxBPThreadCount->setValue(1);
}


void SettingsDialog::on_toolButton_setNGrammModelsDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с моделями NGramm"),
												ui.lineEdit_nGrammModelsDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
	{
		ui.lineEdit_nGrammModelsDir->setText(dir);
		on_toolButton_checkNgarmms_clicked();
	}
}

void SettingsDialog::on_toolButton_setPhonemsModelsDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку с моделями Phonems"),
												ui.lineEdit_phonemsModelsDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
	{
		ui.lineEdit_phonemsModelsDir->setText(dir);
		on_toolButton_checkPhonems_clicked();
	}
}

void SettingsDialog::on_toolButton_setWavDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Установить папку со звуковыми файлами"),
												ui.lineEdit_wavDir->text(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

	if ( !dir.isEmpty() )
	{
		ui.lineEdit_wavDir->setText(dir);
	}
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