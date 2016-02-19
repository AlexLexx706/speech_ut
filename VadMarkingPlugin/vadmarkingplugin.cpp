#include "vadmarkingplugin.h"
#include <assert.h>
#include <iostream>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QFileInfo>
#include <QTextStream>
#include <QtPlugin>

SUVadPlugin::SUVadPlugin():
	//GetProbabilityDouble(NULL),
	//FreeRecognMemory(NULL),
	//lib("SP_UniRec.dll"),
	menu(NULL),
	dialog(NULL),
	isInit(false)
{
}

void SUVadPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_VAD_Plugin");
	actions[1]->setObjectName("action_get_VAD_marking");
	actions[2]->setObjectName("action_get_VAD_marking_on_marking");
	actions[3]->setObjectName("action_start_VAD_batch_processing");
	actions[4]->setObjectName("action_stop_VAD_batch_processing");
}


SUVadPlugin::~SUVadPlugin()
{
	Free();
}


bool SUVadPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
{
	if ( !IsInit() )
	{
		//if ( lib.load() )
		//{
			//GetProbabilityDouble = (pGetProbabilityDouble)lib.resolve("GetProbabilityDouble");
			//FreeRecognMemory = (pFreeRecognMemory)lib.resolve("FreeRecognMemory");

			//if ( FreeRecognMemory && GetProbabilityDouble )
			//{
				if ( inSetting )
				{
					dialog = new SettingsDialog(*inSetting);
					menu = InitActions(inMenu, inToolBar);
				}
				isInit = true;
				return true;
			//}
			//else 
			//	lastErrorDesc = QString::fromUtf8("Неудалось найти функции GetProbabilityDouble и FreeRecognMemory в библиотеке: \"%1\".").arg(lib.fileName());			
		//}
		//else 
		//	lastErrorDesc = QString::fromUtf8("Неудалось загрузить библиотеку: \"%1\".").arg(lib.fileName());						
	}
	else 
		lastErrorDesc = QString::fromUtf8("Плагин уже инициализирован.");
	return false;
}

bool SUVadPlugin::Free()
{
	if ( IsInit() )
	{
		delete dialog;
		dialog = NULL;
		//lib.unload();
		//GetProbabilityDouble = NULL;
		//FreeRecognMemory = NULL;
		delete menu;
		menu = NULL;
		isInit = false;

		return true;
	}
	else 
		lastErrorDesc = QString::fromUtf8("Ресурсы уже освобождены.");	
	return false;
}

void SUVadPlugin::OpenSettingsDialog()
{
	if ( dialog) dialog->exec();
}


QString SUVadPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::mObject parameters;
	parameters["restartProcessing"]=dialog->restartProcessing();
	parameters["processingDir"]=dialog->GetProcessingDir();
	parameters["reportsDir"]=dialog->GetReportsDir();
	parameters["maxBPThreadCount"]=dialog->GetMaxBPThreadCount();
	parameters["useExternalProcessing"]=dialog->UseExternalProcessing();
	parameters["modiffedFilesDir"]=dialog->GetModiffedFilesDir();
	parameters["cutNoice"]=dialog->IsCutNoice();
	parameters["vadParams"]=dialog->GetVadParams();
	parameters["UBMModelPath"]=dialog->GetUBMModelPath();
	parameters["PauseDCModelPath"]=dialog->GetPauseDCModelPath();
	parameters["SpeechDCModelPath"]=dialog->GetSpeechDCModelPath();
	parameters["UseDictorCount"]=dialog->IsUseDictorCount();
	parameters["tresholdParam_dc"]=dialog->GetThresholdDC();
	parameters["LargeWinSize"]=dialog->GetLargeWinSize();
	parameters["cut_models"]=dialog->GetCutModelsDesc();

	json_spirit::mObject topObj;
	topObj["pluginName"] = GetPluginName().toStdString();
	topObj["parameters"] = parameters;

	return QString::fromUtf8(json_spirit::write(topObj, json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool SUVadPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	//std::cout << "GetSettings >" << std::endl;
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.ModiffedFilesDir = QString::fromStdString(params["modiffedFilesDir"].get_str());
			settings.CutNoice = params["cutNoice"].get_bool();
			settings.vadParams = QString::fromUtf8(params["vadParams"].get_str().data());

			settings.UBMModelPath = QString::fromUtf8(params["UBMModelPath"].get_str().data());
			settings.PauseDCModelPath = QString::fromUtf8(params["PauseDCModelPath"].get_str().data());
			settings.SpeechDCModelPath = QString::fromUtf8(params["SpeechDCModelPath"].get_str().data());
			settings.UseDictorCount = params["UseDictorCount"].get_bool();
			settings.tresholdParam_dc = params["tresholdParam_dc"].get_real();
			settings.LargeWinSize = params["LargeWinSize"].get_real();

			json_spirit::mObject & cut_models = params["cut_models"].get_obj();
			settings.cut_models.first = cut_models["use"].get_bool();
			settings.cut_models.second.clear();
			
			json_spirit::mArray & arr = cut_models["models"].get_array();
			
			for(json_spirit::mArray::iterator iter = arr.begin(), end = arr.end(); iter != end; iter++ )
			{
				json_spirit::mObject & obj =iter->get_obj();
				settings.cut_models.second[QString::fromUtf8(obj["name"].get_str().c_str())] = obj["use"].get_bool();
			}
			//std::cout << "GetSettings < ok" << std::endl;
			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromUtf8("Ошибка парсинга настроек: %1").arg(e.what());
	}
	//std::cout << "GetSettings < false;" << std::endl;
	return false;
}


bool SUVadPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
{
	if ( !inData.Signal.size())
	{
		lastErrorDesc = QString::fromUtf8("Фонограмма отсутствует.");
		return false;
	}

	if ( inData.BitsPerSample != 16)
	{
		lastErrorDesc = QString::fromUtf8("bitsPerSample(%1) != 16 ").arg(inData.BitsPerSample);
		return false;
	}

	if ( inData.SampleRate != 8000 )
	{
		lastErrorDesc = QString::fromUtf8("sampleRate(%1) != 8000").arg(inData.SampleRate);
		return false;
	}
	
	try 
	{
		//////////////////////////////////////////////////////////////////////////////
		Settings settings;
		if ( !GetSettings(inData.PluginSettings, settings, lastErrorDesc) )
			return false;


		std::string _errorDesc;
		std::string params(settings.vadParams.toUtf8().data());

		std::list<VAD_GMM::VadSettings> models;
		if ( !VAD_GMM::LoadVadModels(params,  models, _errorDesc) )
		{
			lastErrorDesc = QString::fromStdString(_errorDesc);
			return false;
		}


		outData.Marking.clear();
		bool res = false;

		for ( QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.constBegin();
			iter != inData.Intervals.constEnd(); iter++ )
		{
			std::list< std::pair< std::string, std::pair<int,int>>> marks;
			std::string error;

			if( VAD_GMM::DoVad(models.front(),
					(const unsigned char*)&((short *)inData.Signal.data())[iter->first],
					(iter->second - iter->first) * 2,
					marks,
					error) )
			{
				for ( std::list< std::pair< std::string, std::pair<int,int>>>::iterator mIter = marks.begin(); 
					mIter != marks.end(); mIter++ )
					outData.Marking.append( SUPPhraseData(iter->first + mIter->second.first,
												iter->first + mIter->second.second,
												QString::fromStdString(mIter->first)));
				res = true;
			}
			else
			{
				emit ProgressMessage(id, 0, QString::fromUtf8("Frame(%1, %2) VAD_GMM::DoVad error: %3").
					arg(iter->first).arg(iter->second).arg(QString::fromStdString(error)));
			}
		}
		
		lastErrorDesc = res ? "" : QString::fromUtf8("Неудалось разметить.");
		
		//Подсчёт дикторов.
		if ( res && settings.UseDictorCount )
		{
			HMSData hmsData;
			//грузим модели.
			if ( !LoadHMSModels(hmsData, settings, lastErrorDesc) )
				return false;
			res = HowManySpeakers(inData.Signal, outData.Marking, hmsData,
				settings.tresholdParam_dc, settings.LargeWinSize, lastErrorDesc);

			FreeHMSModels(hmsData);
		}
		return res;
	}
	catch(std::exception & e)
	{
		lastErrorDesc = QString::fromUtf8("Ошибка исполнения: \"%1\".").arg(e.what());
	}
	catch(...)
	{
		lastErrorDesc = QString::fromUtf8("Неизвестная ошибка.");
	}
	return false;
}
bool SUVadPlugin::LoadHMSModels(SUVadPlugin::HMSData & data, const SUVadPlugin::Settings & settings, QString & lastErrorDesc)
{
	//загрузка моделей.

	if ( data.gmmTk.ReadModelFromFile(settings.UBMModelPath.toLocal8Bit().data(), data.UBM) != 0 )
	{
		lastErrorDesc = "Cannot load ubm model";
		return false;
	}

	if ( data.gmmTk.ReadModelFromFile(settings.SpeechDCModelPath.toLocal8Bit().data(), data.speechModel) != 0 )
	{
		lastErrorDesc = "Cannot load speech model";
		data.gmmTk.DisposeMemory(data.UBM);
		return false;
	}

	if ( data.gmmTk.ReadModelFromFile(settings.PauseDCModelPath.toLocal8Bit().data(), data.nonSpeechModel) )
	{
		lastErrorDesc = "Cannot load pause model";
		data.gmmTk.DisposeMemory(data.UBM);
		data.gmmTk.DisposeMemory(data.speechModel);
		return false;
	}
	return true;
}

bool SUVadPlugin::FreeHMSModels(SUVadPlugin::HMSData & data)
{
	if ( data.UBM )
		data.gmmTk.DisposeMemory(data.UBM);
	if ( data.speechModel )
		data.gmmTk.DisposeMemory(data.speechModel);
	if ( data.nonSpeechModel )
		data.gmmTk.DisposeMemory(data.nonSpeechModel);
	return true;
}

bool SUVadPlugin::HowManySpeakers(const QByteArray & signal,
								  QList<SUPPhraseData> & marking,
								  SUVadPlugin::HMSData & hmsData,
								  double tresholdParam_dc,
								  int LargeWinSize,
								  QString & lastErrorDesc)
{
	int commonSize = 0;

	for ( QList<SUPPhraseData>::iterator iter = marking.begin();
		iter != marking.end(); iter++ )
		commonSize += iter->End - iter->Start;

	vector<int> sizeSlices(marking.size());
	std::vector<short> data(commonSize);

	int offset = 0;
	int i = 0;
	std::vector< std::pair<int, int > > marks;
	marks.resize(marking.size());

	for ( QList<SUPPhraseData>::iterator iter = marking.begin();
		iter != marking.end(); iter++ )
	{
		memcpy(&data[offset], &((short *)signal.data())[iter->Start], (iter->End - iter->Start) * 2);
		offset += iter->End - iter->Start;
		sizeSlices[i] = iter->End - iter->Start;
		marks[i].first = iter->Start;
		marks[i].second = iter->End;
		i++;
	}
			
	int outVal = 0;
	vector<int> outClass(marking.size());
	
	int hmsRes = hmsData.sDiSeC.HowManySpeakers(&data[0], 
							&sizeSlices[0], 
							sizeSlices.size(),
							marks,
							hmsData.UBM, 
							hmsData.speechModel, 
							hmsData.nonSpeechModel, 
							tresholdParam_dc,
							outVal, 
							&outClass[0],
							LargeWinSize);

	if ( hmsRes == 0 )
	{
		int index = 0;
		for ( QList<SUPPhraseData>::iterator iter = marking.begin();
			iter != marking.end(); iter++ )
		{
			iter->Name = QString::number(outClass[index]);
			index++;
		}
		return true;
	}
	else 
		lastErrorDesc = QString("sDiSeC.HowManySpeakers error: %1").arg(hmsRes);
	return false;
}



bool SUVadPlugin::InitBatchProcessing(void * & internalData, int id, const QString & settingsStr, QString & errorDesc)
{
	try 
	{
		ThreadData & threadData = *((ThreadData *)(internalData = new ThreadData));

		if ( !GetSettings(settingsStr, threadData.settings, lastErrorDesc) )
			return false;

		std::string _errorDesc;
		std::string params(threadData.settings.vadParams.toUtf8().data());

		std::list<VAD_GMM::VadSettings> models;
		if ( !VAD_GMM::LoadVadModels(params,  models, _errorDesc) )
		{
			lastErrorDesc = QString::fromStdString(_errorDesc);
			return false;
		}
		threadData.vadModel = models.front();


		//////////////////////////////////////////////////////////////////////////////
		if ( threadData.settings.CutNoice )
		{
			//Создадим директорию отчётов.
			QDir mfDir(threadData.settings.ModiffedFilesDir);
			if ( !mfDir.exists() && !mfDir.mkpath(threadData.settings.ModiffedFilesDir) )
			{
				lastErrorDesc = QString::fromUtf8("Неудалось создать директорию для сохранения модиффицированных файлов: \"%1\"").arg(threadData.settings.ModiffedFilesDir);
				return false;
			}
		}

		if ( threadData.settings.UseDictorCount )
		{
			if( !LoadHMSModels(threadData.hsmData, threadData.settings, lastErrorDesc) )
				return false;
		}

		return true;
	}
	catch(...)
	{
		lastErrorDesc = QString::fromUtf8("Unknown exception");
		return false;
	}
}

void SUVadPlugin::RemoveNoiseFromSound(QList<SUPPhraseData> & marking, QByteArray & sound)
{
	QList<SUPPhraseData>::iterator iter = marking.begin();

	while ( iter != marking.end() )
	{
		//Вырезаем фразу из файла, из разметки и смещаем все остальные фразы на длинну данной.
		if ( (iter->Name == "noise") ||
			(iter->Name.size() >= 2 &&  iter->Name[0] == '_' && iter->Name[1] == '_') )
		{
			//! Удалим звук.
			long len = iter->End - iter->Start;
			sound.remove(iter->Start * 2, len * 2);
			iter = marking.erase(iter);

			//! Сместим. разметку. 
			for ( QList<SUPPhraseData>::iterator curIter = iter; 
				curIter != marking.end(); curIter++ )
			{
				curIter->Start = curIter->Start - len;
				curIter->End = curIter->End - len;
			}
		}
		else 
			iter++;
	}
}


void SUVadPlugin::RemoveSignalsFromSound(const QPair<bool, QMap<QString, bool> > & cut_models,
					QList<SUPPhraseData> & marking, 
					QByteArray & sound)
{
	QList<SUPPhraseData> new_marking;

	if ( marking.size() )
	{
		//Создадим временную разметку.
		for ( QList<SUPPhraseData>::iterator iter = marking.begin(), end = marking.end(), next; 
			iter != end; iter++)
		{
			new_marking.append(*iter);
			next = iter; next++;

			if ( next != end )
				new_marking.append(SUPPhraseData(iter->End, next->Start, "pause"));
		}

		//Добавим начало
		if ( new_marking.front().Start > 0 )
			new_marking.push_front(SUPPhraseData(0, new_marking.front().Start, "pause"));

		//Добавим конец
		if ( new_marking.back().End * 2 < sound.size() )
			new_marking.push_back(SUPPhraseData(new_marking.back().End, sound.size()/2, "pause"));

		//for ( QList<SUPPhraseData>::iterator iter = new_marking.begin(), end = new_marking.end(); 
		//	iter != end; iter++)
		//	qDebug() << iter->Start << " "<< iter->End << " " << iter->Name;

		//Вырезаем данные.
		const QMap<QString, bool> & cut_model = cut_models.second;
		QMap<QString, bool>::const_iterator cut_iter;
		QList<SUPPhraseData>::iterator iter = new_marking.begin();
		marking.clear();

		while ( iter != new_marking.end() )
		{
			cut_iter = cut_model.find(iter->Name);
			
			//Вырезаем фразу из файла, из разметки и смещаем все остальные фразы на длинну данной.
			if ( cut_iter != cut_model.end() && (*cut_iter) )
			{
				//! Удалим звук.
				long len = iter->End - iter->Start;
				sound.remove(iter->Start * 2, len * 2);
				iter = new_marking.erase(iter);

				//! Сместим. разметку. 
				for ( QList<SUPPhraseData>::iterator curIter = iter;
							curIter != new_marking.end(); curIter++ )
				{
					curIter->Start = curIter->Start - len;
					curIter->End = curIter->End - len;
				}
			}
			else
			{
				if ( iter->Name != "pause" )
					marking.push_back(*iter);
				iter++;
			}
		}
	}
}


bool SUVadPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & file_report, QString & lastErrorDesc)
{
	QByteArray sound;
	if ( !LoadFile(file_report.first, 16, 8000, sound, lastErrorDesc) )
		return false;

	ThreadData & threadData = *((ThreadData *)internalData);

	std::list< std::pair< std::string, std::pair<int,int>>> marks;
	std::string error;

	QList<SUPPhraseData> marking;

	if( VAD_GMM::DoVad(threadData.vadModel,
										(const unsigned char*)sound.data(),
										sound.size(),			marks,
			error) )
	{
		for ( std::list< std::pair< std::string, std::pair<int,int>>>::iterator mIter = marks.begin(); 
			mIter != marks.end(); mIter++ )
				marking.append( SUPPhraseData(mIter->second.first,
												mIter->second.second,
												QString::fromStdString(mIter->first)));

	}
	else
	{
		lastErrorDesc = QString("VAD_GMM::DoVad error: %1").arg(error.data());
		return false;
	}

	try
	{
		if ( threadData.settings.CutNoice && !threadData.settings.UseDictorCount )
		{
			json_spirit::mValue v;
			bool useVad2 = false;
			if ( json_spirit::read(threadData.settings.vadParams.toUtf8().data(), v) )
				useVad2 = v.get_obj()["vadModels"].get_array()[0].get_obj()["useParams2"].get_bool();

			if ( !useVad2 )
			{
				//Вырезаем шум.
				//std::cout<< "1" << std::endl;
				if ( !threadData.settings.cut_models.first )
				{
					//std::cout<< "2" << std::endl;
					RemoveNoiseFromSound(marking, sound);
					//std::cout<< "3" << std::endl;
				}
				//Режим все модели из списка.
				else
				{
					//std::cout<< "4" << std::endl;
					RemoveSignalsFromSound(threadData.settings.cut_models,marking, sound);
					//std::cout<< "5" << std::endl;
				}

				//! Сохраним новый файл.
				if ( !SaveFile(threadData.settings.ModiffedFilesDir + "/" +
							QFileInfo(file_report.first).fileName(),
							1,
							16,
							8000,
							sound,
							lastErrorDesc) )
					return false;
			}
		else
			{
				QByteArray WNSound;

				QList<SUPPhraseData>::iterator iter = marking.begin();

				//Вырезаем шум.
				while ( iter != marking.end() )
				{
					WNSound.append(sound.mid(iter->Start * 2, (iter->End - iter->Start)*2));
					iter++;
				}
				//! Сохраним новый файл.
				if ( !SaveFile(threadData.settings.ModiffedFilesDir + "/" + QFileInfo(file_report.first).fileName(), 1, 16, 8000, WNSound, lastErrorDesc) )
					return false;
			}
		}
		else if ( threadData.settings.UseDictorCount )
			if ( !HowManySpeakers(sound, marking, threadData.hsmData, 
				threadData.settings.tresholdParam_dc, threadData.settings.LargeWinSize, lastErrorDesc) )
				return false;

		if ( SaveMarkingInFile(file_report.second, marking, lastErrorDesc) )
			return true;
		else
			return false;		
	}
	catch(std::exception & e)
	{
		lastErrorDesc = QString::fromUtf8("Ошибка при разборе результата JSON формата: \"%1\"").arg(e.what());
		return false;
	}
	catch(...)
	{
		lastErrorDesc = QString::fromUtf8("Неизвестная ошибка при разборе результата JSON формата");
		return false;
	}
}

void SUVadPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		ThreadData & threadData = *((ThreadData *)(internalData));
		if ( threadData.settings.UseDictorCount )
			FreeHMSModels(threadData.hsmData);

		delete (ThreadData *)internalData;
		internalData = NULL;
	}
}

Q_EXPORT_PLUGIN2(VadMarkingPlugin, SUVadPlugin);