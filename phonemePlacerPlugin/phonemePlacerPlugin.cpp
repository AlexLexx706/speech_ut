#include "phonemePlacerPlugin.h"
#include <QFileInfo>
#include <iostream>
#include <fstream>
#include "../json_spirit/json_spirit.h"
#include <QMenu>
#include <QToolBar>
#include <QtPlugin>
#include <QDir>
#include <QMessageBox>
#include <Viterbi/phonemePlacer.h>
#include <Preprocessing\ppcEngine.h>


using namespace std;

PhonemePlacerPlugin::PhonemePlacerPlugin():dialog(NULL),menu(NULL),isInit(false)
{}

PhonemePlacerPlugin::~PhonemePlacerPlugin()
{
	Free();
}

void PhonemePlacerPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_PhonemePlacer_Plugin");
	actions[1]->setObjectName("action_get_PhonemePlacer_marking");
	actions[2]->setObjectName("action_get_PhonemePlacer_marking_on_marking");
	actions[3]->setObjectName("action_start_PhonemePlacer_batch_processing");
	actions[4]->setObjectName("action_stop_PhonemePlacer_batch_processing");
}

bool PhonemePlacerPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
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
	lastErrorDesc = QString::fromLocal8Bit("Плагин уже инициализирован.");
	return false;
}

bool PhonemePlacerPlugin::Free()
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
	lastErrorDesc = QString::fromLocal8Bit("Ресурсы плагина уже освобождены.");
	return false;
}

QString PhonemePlacerPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;
	parameters.push_back(json_spirit::Pair("restartProcessing", dialog->restartProcessing()));

	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));

	parameters.push_back(json_spirit::Pair("hmmModel", dialog->GetHmmModel()));
	parameters.push_back(json_spirit::Pair("textFile", dialog->GetTextFileName()));
	parameters.push_back(json_spirit::Pair("transcriptionsDir", dialog->GetTranscriptionsDir()));
	parameters.push_back(json_spirit::Pair("phraseIsCorrect", dialog->PhraseIsCorrect()));
	parameters.push_back(json_spirit::Pair("is_output_type_by_states", dialog->is_output_type_by_states()));


	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool PhonemePlacerPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.HmmModel = QString::fromUtf8(params["hmmModel"].get_str().data());
			settings.TextFileName = QString::fromUtf8(params["textFile"].get_str().data());
			settings.TranscriptionsDir = QString::fromUtf8(params["transcriptionsDir"].get_str().data());
			settings.PhraseIsCorrect = params["phraseIsCorrect"].get_bool();
			settings.is_output_type_by_states = params["is_output_type_by_states"].get_bool();

			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}


bool Preprocess(const short* audio,
				int audioSize, 
				vector<vector<SP_REAL> > & FeatureMap,
				QString & error_desc)
{
	//! Препроцессинг.
	if ( audio && audioSize)
	{
		ppcEngine ppc;
		int errorCode = ppc.PreProcessing(audio, audioSize, FeatureMap, PLP, ZEROMEAN, DELTADELTA, 0);
		if ( errorCode == 0)
			return true;
		error_desc = QString("PreProcessing error: %1").arg(errorCode);
	}
	else
		error_desc = "Bad params";
	return false;
}




bool PhonemePlacerPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
{
	if ( !inData.Signal.size())
	{
		lastErrorDesc = QString::fromLocal8Bit("Фонограмма отсутствует.");
		return false;
	}

	if ( inData.BitsPerSample != 16)
	{
		lastErrorDesc = QString::fromLocal8Bit("bitsPerSample(%1) != 16 ").arg(inData.BitsPerSample);
		return false;
	}

	if ( inData.SampleRate != 8000 )
	{
		lastErrorDesc = QString::fromLocal8Bit("sampleRate(%1) != 8000").arg(inData.SampleRate);
		return false;
	}

	if ( inData.Intervals.size() < 1 )
	{
		lastErrorDesc = QString::fromLocal8Bit("Отсутствует нитервал обработки.");
		return false;
	}

	try 
	{
		Settings settings;
		if ( !GetSettings(inData.PluginSettings, settings, lastErrorDesc) )
			return false;

		std::string transcription;
		//Загрузка транскрипции.
		if ( !LoadTranscription(settings.TextFileName, transcription, lastErrorDesc) )
			return false;

		//Установим модель фонем.
		int model_id;

		QFile f(settings.HmmModel);
		if( f.open(QFile::ReadOnly) &&  CreateModel(f.readAll().data(), 0, &model_id) != 0)
		{
			lastErrorDesc = QString::fromLocal8Bit("Неудалось установить модель(PhonemePlacer_SetPhonemModel) \"%1\"").arg(settings.HmmModel);
			return false;
		}
		f.close();

		//Создадим плейсер.
		int placer_id;
		if ( CreatePlacer(&placer_id) != 0 )
		{
			lastErrorDesc = QString::fromLocal8Bit("Неудалось создать плейсер");
			RemoveModel(model_id);
			return false;
		}


		bool res = false;
		outData.Marking.clear();
		list<pair<string, int> > result;
		bool phraseIsCorrect  = settings.PhraseIsCorrect;
		bool is_output_type_by_states = settings.is_output_type_by_states;
		vector<vector<SP_REAL> > feature_map;
		int dataSize = inData.Intervals[0].second - inData.Intervals[0].first;

		if( Preprocess(&((short *)inData.Signal.data())[inData.Intervals[0].first],
							dataSize,
							feature_map,
							lastErrorDesc) )
		{
			int error_code = Place(placer_id,
									model_id,
									feature_map,
									transcription.c_str(),
									result,
									is_output_type_by_states ? BYSTATE : BYPHONEME,
									&phraseIsCorrect);

			//Передадим дополнительное описание результата.
			outData.AdditionData = CreateAdditionResult(phraseIsCorrect);

			if (error_code >= 0)
			{
				AddMarking(dataSize, result, inData.Intervals[0].first, outData.Marking);
				res = true;
			}
			else
				lastErrorDesc = QString("Can't get marking, error: %1").arg(GetLastError(placer_id));
		}

		RemoveModel(model_id);
		RemovePlacer(placer_id);
		return res;
	}
	catch(...)
	{
		lastErrorDesc = "Unknown error";
	}
	return false;
}

bool PhonemePlacerPlugin::PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc)
{
	if ( result )
	{
		try 
		{
			json_spirit::mValue v;
			if ( json_spirit::read(data.AdditionData.toUtf8().data(), v) )
			{
				if ( v.get_obj()["PluginName"] == GetPluginName().toStdString()  ) 
				{
					if ( v.get_obj()["Result"].get_obj()["PhraseIsCorrect"] == false )
					{
						if( QMessageBox::question(NULL, 
									QString::fromLocal8Bit("Внимание"),
									QString::fromLocal8Bit("Вероятно в фонограмме звучит фраза отличающаяся от транскрипции. Показать разметку?"),
									QMessageBox::Yes | QMessageBox::No,
									QMessageBox::Yes) == QMessageBox::No )
						{
							return false;
						}
					}
				}
			}
		}
		catch( std::exception & e)
		{
			errorDesc = QString::fromLocal8Bit("Ошибка формата JSON error: %1").arg(e.what());
			return false;
		}
	}
	return result;
}

bool PhonemePlacerPlugin::BeginBatchProcessing(int id, const QString & in_str_settings, QString & errorDesc)
{
	Settings settings;
	
	if ( !GetSettings(in_str_settings, settings, errorDesc) )
		return false;

	if ( !QDir(settings.TranscriptionsDir).exists() )
	{
		errorDesc = QString::fromLocal8Bit("Директория с транскрипциями не существует \"%1\"").arg(settings.TranscriptionsDir);
		return false;
	}

	int model_id;

	QFile f(settings.HmmModel);

	if( !f.open(QFile::ReadOnly) )
	{
		errorDesc = QString::fromLocal8Bit("Неудалось прочитать файл модели:\"%1\"").arg(settings.HmmModel);
		return false;
	}
	
	if ( CreateModel(f.readAll().data(), 0, &model_id) != 0)
	{
		errorDesc = QString::fromLocal8Bit("Неудалось создать модель:\"%1\"").arg(settings.HmmModel);
		return false;
	}
	QMutexLocker ml(&model_map_locker);
	model_map[id] = model_id;

	return true;
}

bool PhonemePlacerPlugin::GetModelId(int id, int & model_id, bool remove_id)
{
	QMutexLocker ml(&model_map_locker);
	QMap<int, int>::iterator iter = model_map.find(id);
	
	if ( iter != model_map.end() )
	{
		model_id = *iter;
		if ( remove_id )
			model_map.erase(iter);
		return true;
	}
	return false;
}

void PhonemePlacerPlugin::EndBatchProcessing(int id)
{
	int model_id;

	if ( GetModelId(id, model_id, true) )
		RemoveModel(model_id);
}

bool PhonemePlacerPlugin::InitBatchProcessing(void * & internalData,
											int id,
											const QString & str_settings,
											QString & errorDesc)
{
	BatchThreadData & threadData  = *((BatchThreadData *)(internalData = new BatchThreadData));

	if ( !GetSettings(str_settings, threadData.settings, errorDesc) )
		return false;

	threadData.placer_id = -1;

	if ( CreatePlacer(&threadData.placer_id) != 0 )
	{
		errorDesc = QString::fromLocal8Bit("Неудаётся создать плейсер");
		return false;
	}

	return true;
}

void PhonemePlacerPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	BatchThreadData & threadData  = *((BatchThreadData *)(internalData));

	if ( threadData.placer_id != -1 )
		RemovePlacer(threadData.placer_id);
	delete (BatchThreadData *)(internalData);
}


bool PhonemePlacerPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & lastErrorDesc)
{
	BatchThreadData & threadData  = *((BatchThreadData *)internalData);
	try 
	{
		std::string transcription;
		QString baseName = QFileInfo(fileDesc.first).completeBaseName();
		QString fileName = threadData.settings.TranscriptionsDir + "/" + baseName + ".lab";

		QString strErr;
		//! Загружаем транскрипцию.
		if ( !LoadTranscription(fileName, transcription, strErr) )
		{
			QString fileName = threadData.settings.TranscriptionsDir + "/" + baseName + ".txt";
			if ( !LoadTranscription(fileName, transcription, strErr) )
			{
				lastErrorDesc = strErr;
				return false;
			}
		}

		//! загрузка файла.
		QByteArray signal;
		if ( !LoadFile(fileDesc.first, 16, 8000, signal, lastErrorDesc) )
			return false;

		//! расстановка фонем.
		list<pair<string, int> > result;
		bool phraseIsCorrect  = threadData.settings.PhraseIsCorrect;
		bool is_output_type_by_states = threadData.settings.is_output_type_by_states;






		vector<vector<SP_REAL> > feature_map;

		if( Preprocess((const short *)signal.data(),
							signal.size()/2,
							feature_map,
							lastErrorDesc) )
		{
			int model_id;
			GetModelId(id, model_id,false);

			int error_code = Place(threadData.placer_id,
									model_id,
									feature_map,
									transcription.c_str(),
									result,
									is_output_type_by_states ? BYSTATE : BYPHONEME,
									&phraseIsCorrect);
			
			if ( threadData.settings.PhraseIsCorrect != phraseIsCorrect )
			{
				lastErrorDesc = QString::fromLocal8Bit("Неудалось разметить, файл содержит другие фонемы.");
				return false;
			}

			if ( error_code == 0 )
			{
				QList<SUPPhraseData> marking;
				AddMarking(signal.size()/2, result, 0, marking);
				return SaveMarkingInFile(fileDesc.second, marking, lastErrorDesc);
			}
			else
			{
				lastErrorDesc = QString::fromLocal8Bit("Неудалось получить разметку, error: %1 ").
					arg(GetLastError(threadData.placer_id));
			}
		}
		return false;
	}
	catch(std::exception & e)
	{
		lastErrorDesc = QString("Exception: %1").arg(e.what());
	}
	catch(...)
	{
		lastErrorDesc = QString("Uncknown exception");
	}
	return false;
}


bool PhonemePlacerPlugin::LoadTranscription(const QString & fileName, std::string & transcription, QString & lastErrorDesc)
{
	QFile file(fileName);
	if ( file.open(QIODevice::ReadOnly) )
	{
		QString str = file.readAll();
		str.replace(QRegExp("\\d+"), "");
		QStringList list = str.split(QRegExp("\\s+"));
		for (QStringList::iterator iter = list.begin(); 
				iter != list.end(); iter++ )
		{
			QStringList::iterator next = iter;
			next++;
			if (next != list.end())
				transcription += iter->toStdString() + " ";
			else
				transcription += iter->toStdString();
		}

		if ( !transcription.size() )
			lastErrorDesc = QString("Can't read transcription file: \"%1\"").arg(fileName);
		else 
			return true;
	}
	else 
		lastErrorDesc = QString("Can't read transcription : \"%1\"").arg(fileName);
	return false;
}

QString PhonemePlacerPlugin::CreateAdditionResult(bool phraseIsCorrect)
{
	//Передадим дополнительное описание результата.
	json_spirit::Object obj;
	obj.push_back(json_spirit::Pair("PhraseIsCorrect", dialog->PhraseIsCorrect() == phraseIsCorrect));
	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("PluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("Result", obj));
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

void PhonemePlacerPlugin::AddMarking(long framesCount, const std::list<std::pair<std::string, int> > & result, long offset, QList<SUPPhraseData> & marking)
{
	for ( list<pair<string, int> >::const_iterator iter = result.begin();
		iter != result.end(); iter++ )
	{
		list<pair<string, int> >::const_iterator next = iter;
		next++;
		int start = iter->second * 80;
		int end = next != result.end() ? next->second * 80: framesCount;
		marking.append(SUPPhraseData(start + offset, end + offset, QString::fromStdString(iter->first)));
	}
}


Q_EXPORT_PLUGIN2(PhonemePlacerPlugin, PhonemePlacerPlugin);