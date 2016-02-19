#include "LanguageIdentificationPlugin.h"
#include <QFileInfo>
#include <iostream>
#include <fstream>
#include "../json_spirit/json_spirit.h"
#include <QMenu>
#include <QToolBar>
#include <QtPlugin>
#include <QDir>
#include <QMessageBox>
#include "../Common/soundrecorder/soundrecorder.h"
#include "QDebug"
#include "Language Identification/Recognition.h"


using namespace std;


LanguageIdentificationPlugin::LanguageIdentificationPlugin():dialog(NULL),menu(NULL),isInit(false),serverCount(0)
{}

LanguageIdentificationPlugin::~LanguageIdentificationPlugin()
{
	Free();
}

void LanguageIdentificationPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_LanguageIdentificationPlugin");
	actions[1]->setObjectName("action_start_LanguageIdentificationPlugin_batch_processing");
	actions[2]->setObjectName("action_stop_LanguageIdentificationPlugin_batch_processing");
}

bool LanguageIdentificationPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
{
	if ( !IsInit() )
	{
		if ( inSetting )
		{
			dialog = new SettingsDialog(*inSetting);
			menu = InitActions(inMenu, inToolBar);
		}
		isInit = true;
		return true;
	}
	lastErrorDesc = QString::fromUtf8("Плагин уже инициализирован.");
	return false;
}

bool LanguageIdentificationPlugin::Free()
{
	if ( IsInit() )
	{
		delete dialog;
		dialog = NULL;
		delete menu;
		menu = NULL;
		isInit = false;
		return true;
	}
	lastErrorDesc = QString::fromUtf8("Ресурсы плагина уже освобождены.");
	return false;
}

QString LanguageIdentificationPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::mObject parameters;


	parameters["useMoreServer"]= dialog->UseMoreServer();
	parameters["moreServerHost"]= dialog->MoreServerHost();
	parameters["moreServerPort"]= dialog->MoreServerPort();


	parameters["processingDir"]= dialog->GetProcessingDir();
	parameters["maxBPThreadCount"]= dialog->GetMaxBPThreadCount();
	parameters["useExternalProcessing"]= dialog->UseExternalProcessing();
	parameters["reportsDir"]= dialog->GetOutDir();
	parameters["restartProcessing"] = dialog->RestartProcessing();

	parameters["saveRecognitionReport"]= dialog->SaveRecognitionReport();
	parameters["sortFilesByLangs"]= dialog->SortFilesByLangs();
	parameters["createLabFile"]= dialog->CreateLabFile(); 



	parameters["pauseModelPath"]= dialog->GetPauseModelPath();
	parameters["speechModelPath"]= dialog->GetSpeechModelPath();
	parameters["modelsDir"]= dialog->GetModelsDir();
	parameters["modelsMask"]= dialog->GetModelsMask();

	parameters["nGrammModelsPath"]=  dialog->GetNGrammModelsPath();
	parameters["nGrammModelsMask"]=  dialog->GetNGrammModelsMask();
	parameters["nGrammModelsMask_aux"]=  dialog->GetNGrammModelsMaskAux();

	parameters["phonemsModelsPath"]=  dialog->GetPhonemsModelsPath();
	parameters["phonemsModelsMask"]=  dialog->GetPhonemsModelsMask();
	parameters["noPhonemeChannels"]=  dialog->noPhonemeChannels();

	parameters["useVad"]=  dialog->UseVad();
	
	parameters["useTxtModel"]=  dialog->useTxtModel();
	parameters["grammCount"]=  dialog->ngrammCount();
	parameters["threshold"]=  dialog->threshold();


	QPair<QString, QStringList> langs = dialog->GetCurLanguages();
	parameters["ngrammChannel"] = langs.first.toUtf8().data();
	
	json_spirit::mArray arr;
	foreach(const QString & l, langs.second)
		arr.push_back(l.toUtf8().data());
	parameters["ngrammModels"]= arr;

	QPair<QString, QStringList> phonems = dialog->GetPhonems();
	parameters["phonemsChannel"] = phonems.first.toUtf8().data();
	
	arr.clear();
	foreach(const QString & p, phonems.second)
		arr.push_back(p.toUtf8().data());
	parameters["phonemsModels"]= arr;
	parameters["useOpenTask"] = dialog->useOpenTask();
	


	json_spirit::mObject topObj;
	topObj["pluginName"] = GetPluginName().toUtf8().data();
	topObj["parameters"] = parameters;
	return QString::fromUtf8(json_spirit::write(topObj, json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool LanguageIdentificationPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();
	
			
			
			settings.useMoreServer = params["useMoreServer"].get_bool();
			settings.moreServerHost = QString::fromUtf8(params["moreServerHost"].get_str().data());
			settings.moreServerPort = params["moreServerPort"].get_int();


			settings.outDir = QString::fromUtf8(params["reportsDir"].get_str().data());

			settings.saveRecognitionReport = params["saveRecognitionReport"].get_bool();
			settings.sortFilesByLangs = params["sortFilesByLangs"].get_bool();
			settings.createLabFile = params["createLabFile"].get_bool();

			settings.pauseModelPath = QString::fromUtf8(params["pauseModelPath"].get_str().data());
			settings.speechModelPath = QString::fromUtf8(params["speechModelPath"].get_str().data());
			settings.modelsDir = QString::fromUtf8(params["modelsDir"].get_str().data());
			settings.modelsMask = QString::fromUtf8(params["modelsMask"].get_str().data());

			settings.nGrammModelsPath = QString::fromUtf8(params["nGrammModelsPath"].get_str().data());
			settings.nGrammModelsMask = QString::fromUtf8(params["nGrammModelsMask"].get_str().data());
			settings.nGrammModelsMask_aux = QString::fromUtf8(params["nGrammModelsMask_aux"].get_str().data());


			settings.phonemsModelsPath = QString::fromUtf8(params["phonemsModelsPath"].get_str().data());
			settings.phonemsModelsMask = QString::fromUtf8(params["phonemsModelsMask"].get_str().data());
			settings.noPhonemeChannels = params["noPhonemeChannels"].get_bool();

			settings.useVad = params["useVad"].get_bool();

			settings.useTxtModel = params["useTxtModel"].get_bool();
			settings.grammCount  = params["grammCount"].get_int();
			settings.threshold  = params["threshold"].get_real();
			settings.useOpenTask = params["useOpenTask"].get_bool();



			//Создадим список моделей ngramm.
			settings.languages.channel = params["ngrammChannel"].get_str();
			settings.languages.models.clear();
			settings.languages.__models.clear();

			json_spirit::mArray & lArr = params["ngrammModels"].get_array();
			
			for( json_spirit::mArray::iterator iter = lArr.begin();
				iter != lArr.end(); iter++ )
			{
				settings.languages.__models.push_back(iter->get_str());
				settings.languages.models.push_back(settings.languages.__models.back().c_str());
			}

			//Создадим список моделей phonems.
			settings.phonems.channel = params["phonemsChannel"].get_str();
			settings.phonems.models.clear();
			settings.phonems.__models.clear();

			lArr = params["phonemsModels"].get_array();
			
			for( json_spirit::mArray::iterator iter = lArr.begin();
				iter != lArr.end(); iter++ )
			{
				settings.phonems.__models.push_back(iter->get_str());
				settings.phonems.models.push_back(settings.phonems.__models.back().c_str());
			}

			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromUtf8("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}

bool LanguageIdentificationPlugin::PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc)
{
	return result;
}

bool LanguageIdentificationPlugin::BeginBatchProcessing(int id, const QString & settings, QString & errorDesc)
{
	batchLocker.lock();
	BatchThreadData & threadData = batchProcDescs[id];
	batchLocker.unlock();

	return InitRecognition(true,
							settings,
							threadData.settings,
							threadData.client,
							threadData.serverID,
							errorDesc);
}

void LanguageIdentificationPlugin::EndBatchProcessing(int id)
{
	batchLocker.lock();
	BatchThreadData & desc = batchProcDescs[id];
	batchLocker.unlock();
	Recognition::DeleteInstance(desc.client);
}


bool LanguageIdentificationPlugin::InitBatchProcessing(void * & internalData,
													   int id,
													   const QString & settingsStr,
													   QString & errorDesc)
{
	batchLocker.lock();
	BatchThreadData & desc = batchProcDescs[id];
	batchLocker.unlock();

	internalData = new int;
	Recognition::CreateInstance(((int*)internalData));

	if ( Recognition::Init(*((int*)internalData), 
							NULL,
							NULL,
							desc.serverID) != 1 )
	{
		errorDesc = Recognition::GetLastErrorDesc(*((int*)internalData));
		return false;
	}
	return true;
}

void LanguageIdentificationPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		Recognition::DeleteInstance(*((int *)internalData));
		delete (int *)internalData;
		internalData = NULL;
	}
}




bool LanguageIdentificationPlugin::InitRecognition(bool batchProcessing,
												   const QString & settingsStr,
												   Settings & settings,
												   int & outClientId,
												   int & serverID,
												   QString & errorDesc)
{
	if ( !GetSettings(settingsStr, settings, errorDesc) )
		return false;

	//Создадим выходную директорию
	QDir dir(settings.outDir);

	if ( batchProcessing && !dir.exists() && !QDir().mkpath(settings.outDir) )
	{
		errorDesc = QString::fromUtf8("Неудалось создать выходную директорию:\"%1\"").arg(settings.outDir);
		return false;
	}
	if ( settings.languages.channel.empty() )
	{
		errorDesc = QString::fromUtf8("Канал языков не установлен");
		return false;
	}
	
	if ( !settings.languages.models.size() )
	{
		errorDesc = QString::fromUtf8("Языки не выбраны");
		return false;
	}

	if ( settings.phonems.channel.empty() )
	{
		errorDesc = QString::fromUtf8("Канал распознователей не установлен");
		return false;
	}


	if ( !settings.phonems.models.size() )
	{
		errorDesc = QString::fromUtf8("распознователи не выбраны");
		return false;
	}

	if ( !settings.saveRecognitionReport && !settings.sortFilesByLangs && 
		!settings.createLabFile )
	{
		errorDesc = QString::fromUtf8("Неустановлены варианты сохранения результатов");
	}

	Recognition::CreateInstance(&outClientId);
	
	batchLocker.lock();
	serverID = serverCount;
	serverCount++;
	batchLocker.unlock();

	if( settings.useMoreServer )
	{
		if( Recognition::Init(outClientId, 
						settings.moreServerHost.toStdString().c_str(),
						settings.moreServerPort,
						serverID) != 1 )
		{
			errorDesc = QString::fromUtf8("Recognition::Init error");
			return false;
		}
	}
	else
	{
		if ( Recognition::Init(outClientId,0, 0, serverID) != 1 )
		{
			errorDesc = QString::fromUtf8("Recognition::Init error");
			return false;
		}
	}

	//1 Инициализация вада.
	if ( !loadVADmodels(settings.pauseModelPath,
					settings.speechModelPath,
					settings.modelsDir,
					settings.modelsMask,
					outClientId,
					errorDesc) )
	{
		return false;
	}

	//qDebug() << "start LoadPhonemsModels";

	//2. Загрузка моделей фонем.
	if ( !SettingsDialog::LoadPhonemsModels(settings.noPhonemeChannels,
											settings.phonemsModelsPath,
											settings.phonemsModelsMask,
											QMap<QString, QStringList>(),
											errorDesc,
											outClientId,
											true) )
		return false;


	//qDebug() << "start LoadNGrammModels";
	//3. Загрузка моделей NGramm
	if ( !SettingsDialog::LoadNGrammModels(settings.nGrammModelsPath,
								settings.nGrammModelsMask,
								settings.nGrammModelsMask_aux,
								QMap<QString, QStringList>(),
								errorDesc,
								outClientId,
								true,
								settings.useTxtModel) )
		return false;

	//4 создадим папки для сортировки
	if ( batchProcessing && settings.sortFilesByLangs )
	{
		settings.losersDirName = "";
		for(int i = 0; i < settings.languages.models.size(); i++ )
		{
			//Создадим директорию отчётов.
			QDir dir(settings.outDir + "/" + QString(settings.languages.models[i]));
			
			if ( !dir.exists() && !dir.mkpath(".") )
			{
				errorDesc = QString::fromUtf8("Неудалось создать директорию \"%1\" языка \"%2\"").
					arg(dir.path()).arg(QString(settings.languages.models[i]));
				return false;
			}
			settings.losersDirName += QString("!%1_").arg(settings.languages.models[i]);
		}

		QDir dir2(settings.outDir + "/" + settings.losersDirName);
		
		if ( !dir2.exists() && !dir2.mkpath(".") )
		{
			errorDesc = QString::fromUtf8("Неудалось создать директорию \"%1\" языка \"%2\"").
				arg(dir2.path()).arg(QString("!%1").arg(settings.losersDirName));
			return false;
		}
	}
	return true;
}

bool LanguageIdentificationPlugin::FileProcessing(void * internalData,
												  int id,
												  const QPair<QString, QString> & fileDesc,
												  QString & lastErrorDesc)
{
	batchLocker.lock();
	Settings & settings  = batchProcDescs[id].settings;
	batchLocker.unlock();
	int & clientID = *((int *)internalData);


	//грузим файл.
	QByteArray sound;
	if ( !LoadFile(fileDesc.first, 16, 8000, sound, lastErrorDesc) )
		return false;

	int err_code = Recognition::RecognizeItem(clientID,
												settings.grammCount,
												reinterpret_cast<const short *>(sound.data()),
												sound.size()/2,
												NULL,
												settings.useVad ? "vad1" : NULL,
												settings.languages.channel.c_str(),
												&settings.languages.models[0],
												settings.languages.models.size(),

												settings.phonems.channel.c_str(),
												&settings.phonems.models[0],
												settings.phonems.models.size());
	if( err_code != 1 )
	{
		lastErrorDesc = QString::fromUtf8("Ошибка распознователя:%1").arg(err_code);
		return false;
	}
	else
	{
		return SaveRes(Recognition::GetRecognitionResult(clientID),
														fileDesc,
														sound,
														settings,
														lastErrorDesc);
	}
}

bool LanguageIdentificationPlugin::GetWinner(const std::string & pRecResults,
											std::list<std::string> & winners,
											std::list<std::string> & losers,
											QString & lastErrorDesc,
											std::string & outReport,
											const Settings & settings)
{
	json_spirit::mValue v;
	winners.clear();
	losers.clear();

	try
	{
		if( json_spirit::read(pRecResults, v) )
		{
			json_spirit::mArray & arr = v.get_obj()["list of probabilities"].get_array();
			
			if ( settings.useOpenTask )
			{
				double probability;

				for ( json_spirit::mArray::iterator iter = arr.begin();
					iter != arr.end(); iter++ )
				{
					std::stringstream(iter->get_obj()["probability"].get_str()) >> probability;

					if ( probability >= settings.threshold )
						winners.push_back(iter->get_obj()["language"].get_str());
					else
						losers.push_back("!" + iter->get_obj()["language"].get_str());
				}
			}
			else
			{
				std::string language;
				bool _init = false;
				double probability;
				double ___probability;
				
				for ( json_spirit::mArray::iterator iter = arr.begin();
					iter != arr.end(); iter++ )
				{
					std::stringstream(iter->get_obj()["probability"].get_str()) >> ___probability;
					if (!_init)
					{
						language = iter->get_obj()["language"].get_str();
						probability = ___probability;
						_init = true;
					}
					else
					{

						if ( ___probability > probability )
						{
							probability = ___probability;
							language = iter->get_obj()["language"].get_str();
						}
					}
				}
				winners.push_back(language);
			}

			json_spirit::mArray t_arr;
			t_arr.resize(winners.size()); 
			std::copy(winners.begin(), winners.end(), t_arr.begin());
			v.get_obj()["winner"] = t_arr;

			t_arr.resize(losers.size()); 
			std::copy(losers.begin(), losers.end(), t_arr.begin());
			v.get_obj()["losers"] = t_arr;


			outReport = json_spirit::write(v,json_spirit::pretty_print | json_spirit::raw_utf8);
			return true;
		}
		else
			throw 1;
	}
	catch(...)
	{
		lastErrorDesc = QString::fromUtf8("Ошибка парсинга результата рекогнишина");
		return false;
	}
	return false;
}

bool LanguageIdentificationPlugin::SaveRes(const std::string & pRecResults,
											const QPair<QString, QString> & fileDesc,
											const QByteArray & sound,
											const Settings & settings,
											QString & lastErrorDesc)
{
	std::string outReport;
	std::list<std::string> winners;
	std::list<std::string> losers;

	if ( !GetWinner(pRecResults, winners, losers, lastErrorDesc, outReport, settings) )
		return false;


	if ( settings.saveRecognitionReport )
	{
		QFile file(fileDesc.second);
		
		if( file.open(QIODevice::WriteOnly) )
		{
			file.write(outReport.data());
			file.close();
		}
		else
		{
			lastErrorDesc = QString::fromUtf8("Неудалось создать файл результата:\"%1\"").arg(fileDesc.second);
			return false;
		}
	}

	if ( settings.sortFilesByLangs )
	{
		if (!winners.size())
		{
			QString newFilePath(settings.outDir + 
								"\\" + 
								settings.losersDirName + 
								"\\" +
								QFileInfo(fileDesc.first).fileName());
			if( QFile::exists(newFilePath) )
				QFile::remove(newFilePath);

			if( !QFile::copy(fileDesc.first, newFilePath) )
			{
				lastErrorDesc = QString::fromUtf8("Неудалось скопировать файл из \"%1\" в \"%2\"").
					arg(fileDesc.first).arg(newFilePath);
				return false;
			}
		}

		foreach(const std::string & language, winners)
		{
			QString newFilePath(settings.outDir + 
								"\\" + 
								QString::fromStdString(language) + 
								"\\" +
								QFileInfo(fileDesc.first).fileName());


			if( QFile::exists(newFilePath) )
				QFile::remove(newFilePath);

			if( !QFile::copy(fileDesc.first, newFilePath) )
			{
				lastErrorDesc = QString::fromUtf8("Неудалось скопировать файл из \"%1\" в \"%2\"").
					arg(fileDesc.first).arg(newFilePath);
				return false;
			}
		}

	}

	if ( settings.createLabFile )
	{
		QFileInfo info(fileDesc.first);
		
		std::string language;

		foreach(const std::string & lang, winners)
			language = language + "+" + lang;
		
		if( !SaveMarkingInFile(settings.outDir + "/" + info.completeBaseName() + ".lab",
						QList<SUPPhraseData>() << SUPPhraseData(0, sound.size()/2, QString::fromStdString(language)),
						lastErrorDesc) )
		{
			return false;
		}
	}

	return true;
}

bool LanguageIdentificationPlugin::getModels(QString path,
											QString mask,
											QByteArray & bufferInModels,
											QString & jSonPar,
											QString & errorDesc)
{
	int totalFileSize = 0;

	QDir indir(path);
	if(indir.exists())
	{
		QString model;
		QStringList inlist = indir.entryList(QStringList()<<mask, QDir::Files|QDir::NoDotAndDotDot);
		if (inlist.isEmpty())
		{
			errorDesc = QString::fromUtf8("Папка \"%1\" с VAD моделями пуста").arg(path);
			return false;
		}

		int count = 0;
		std::vector <QString> tmp;
		foreach(model, inlist)
		{
			tmp.push_back(model);
			QString fileToProcceed = path + "/" + model;
			QFile frModel(fileToProcceed);
			
			if (!frModel.open(QIODevice::ReadOnly))
			{
				errorDesc = QString::fromUtf8("Неудалось открыть файл модела VAD:\"%1\"").arg(frModel.fileName());
				return false;
			}

			totalFileSize += frModel.size();
		}
		bufferInModels.resize(totalFileSize); 
		totalFileSize = 0;
		jSonPar = "{\"models\": [";
		foreach(model, inlist)
		{
			QString fileToProcceed = path + "/" + model;
			///////////////////////////////////////////////////////////////////////////////
			int fileSizeTmp = 0;
			QFile frOut(fileToProcceed);
			
			if(!frOut.open(QIODevice::ReadOnly))
			{
				errorDesc = QString::fromUtf8("Неудалось открыть файл модела VAD:\"%1\"").arg(frOut.fileName());
				return false;
			}


			memcpy_s(bufferInModels.data() + totalFileSize, frOut.size(), frOut.readAll().data(), frOut.size());
			totalFileSize += frOut.size();

			count++;
			if (count == 1)
				jSonPar +=  "{\"dataSize\": 15376, \"type\": \"type1\", \"name\": \"" + model + "\"}";
			else
				jSonPar +=  ", {\"dataSize\": 15376, \"type\": \"type1\", \"name\": \"" + model + "\"}";
		}
		/////////////////////////////////////////////////////////////////////////////// 
	}
	else
	{
		errorDesc = QString::fromUtf8("Папка \"%1\" с VAD моделями не существует").arg(path);
		return false;
	}

	jSonPar += "],";
	return true;
}

bool LanguageIdentificationPlugin::loadVADmodels(const QString & pauseModelPath,
												const QString & speechModelPath,
												const QString & modelsPath,
												const QString & masks,
												int clientID,
												QString & errorDesc)
{
	//////////////////// Read pause model ///////////////////////////////
	
    QFile filePause(pauseModelPath);
	if(!filePause.open(QIODevice::ReadOnly))
	{
		errorDesc = QString::fromUtf8("Неудалось загрузить файл модели паузы: \"%1\"").arg(pauseModelPath);
		return false;
	}

	QByteArray pause(filePause.readAll());

	//////////////////// Read speech model //////////////////////////////
	QFile fileSpeech(speechModelPath);
	if(!fileSpeech.open(QIODevice::ReadOnly))
	{
		errorDesc = QString::fromUtf8("Неудалось загрузить файл модели речи: \"%1\"").arg(speechModelPath);
		return false;
	}

	QByteArray speech(fileSpeech.readAll());

    QByteArray bufferInModels;
    QString params;

    if( !getModels(modelsPath,
			masks,
			bufferInModels,
			params,
			errorDesc) )
			return false;
    params += "\"StepWin\": \"0.010\", \"Version\": \"1.0\", \"MinFreq\": 50,"
		" \"SampleRate\": 8000, \"WavType\": 1, \"BitsPerSample\": 16, \"Channels\": 1,"
		"\"NumBankFilters\": 26, \"CepLiftering\": \"22.0\", \"MaxFreq\": 4000, \"normalize\" : 0,"
		"\"DurWin\": \"0.025\", \"PreEmphCoeff\": \"0.97\"}";
	
	if( Recognition::AddVADModel(clientID, "vad1",
								pause.data(),
								pause.size(),
								speech.data(),
								speech.size(),
								bufferInModels.data(),
								bufferInModels.size(),
								params.toStdString().c_str()) != 1)
	{
		errorDesc = "Recognition::AddVADModel error";
		return false;
	}
	return true;
}


bool LanguageIdentificationPlugin::GetMarking(int id,
											  const SUPInputData & inData,
											  SUPOutData & outData,
											  QString & lastErrorDesc)
{
	Settings settings;
	int clientID;
	int serverID;
	
	if( InitRecognition(false, 
						inData.PluginSettings,
						settings,
						clientID,
						serverID,
						lastErrorDesc) )
	{
		outData.Marking.clear();
		bool res = true;
		lastErrorDesc = "";

		for ( QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.constBegin();
			iter != inData.Intervals.constEnd(); iter++ )
		{
			//initLocker.lock();

			int pRecResultsSize;

			int err_code = Recognition::RecognizeItem(clientID,
													settings.grammCount,
													&((short *)inData.Signal.data())[iter->first],
													iter->second - iter->first,
													0,
													settings.useVad ? "vad1" : NULL,
													settings.languages.channel.c_str(),
													&settings.languages.models[0],
													settings.languages.models.size(),
													settings.phonems.channel.c_str(),
													&settings.phonems.models[0],
													settings.phonems.models.size());

			if ( err_code != 1 )
			{
				lastErrorDesc = QString::fromLocal8Bit("Frame(%1, %2) Recognition::RecognizeItem"
															" error: %3").arg(iter->first).
																		  arg(iter->second).
																		  arg(err_code);
				emit ProgressMessage(id, 0, lastErrorDesc);
				res = false;
			}
			else
			{
				std::string language;
				std::string outReport;
				std::list<std::string> winners,losers;

				bool r = GetWinner(Recognition::GetRecognitionResult(clientID),
					               winners,
								   losers,
								   lastErrorDesc,
								   outReport,
								   settings);
				
				foreach(const std::string & lang, winners)
					language = language + "+" + lang;

				if( r )
				{
					outData.Marking.append(SUPPhraseData(iter->first,
														iter->second,
														QString::fromStdString(language)));
				}
				else
					emit ProgressMessage(id, 0, lastErrorDesc);
			}
		}
		Recognition::DeleteInstance(clientID);
		return res;
	}
	return false;
}





Q_EXPORT_PLUGIN2(LanguageIdentificationPlugin, LanguageIdentificationPlugin);