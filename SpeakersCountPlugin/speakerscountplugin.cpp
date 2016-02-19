#include "speakerscountplugin.h"
#include "../json_spirit/json_spirit.h"
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



SpeakersCountPlugin::SpeakersCountPlugin():
	//GetProbabilityDouble(NULL),
	//FreeRecognMemory(NULL),
	//lib("SP_UniRec.dll"),
	menu(NULL),
	dialog(NULL),
	isInit(false)
{
}

void SpeakersCountPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_SpeakersCount_Plugin");
	actions[1]->setObjectName("action_get_SpeakersCount_marking");
	actions[2]->setObjectName("action_get_SpeakersCount_marking_on_marking");
	actions[3]->setObjectName("action_start_SpeakersCount_batch_processing");
	actions[4]->setObjectName("action_stop_SpeakersCount_batch_processing");
}


SpeakersCountPlugin::~SpeakersCountPlugin()
{
	Free();
}


bool SpeakersCountPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
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
			//	lastErrorDesc = QString::fromLocal8Bit("Неудалось найти функции GetProbabilityDouble и FreeRecognMemory в библиотеке: \"%1\".").arg(lib.fileName());			
		//}
		//else 
		//	lastErrorDesc = QString::fromLocal8Bit("Неудалось загрузить библиотеку: \"%1\".").arg(lib.fileName());						
	}
	else 
		lastErrorDesc = QString::fromLocal8Bit("Плагин уже инициализирован.");
	return false;
}

bool SpeakersCountPlugin::Free()
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
		lastErrorDesc = QString::fromLocal8Bit("Ресурсы уже освобождены.");	
	return false;
}



QString SpeakersCountPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;
	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));

	parameters.push_back(json_spirit::Pair("ubmModelPath", dialog->GetUbmModelPath()));
	parameters.push_back(json_spirit::Pair("speechModelPath", dialog->GetSpeechModelPath()));
	parameters.push_back(json_spirit::Pair("pauseModelPath", dialog->GetPauseModelPath()));
	parameters.push_back(json_spirit::Pair("tresholdParam", dialog->GetTresholdParam()));


	

	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool SpeakersCountPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.ubmModelPath = QString::fromUtf8(params["ubmModelPath"].get_str().data());
			settings.speechModelPath = QString::fromUtf8(params["speechModelPath"].get_str().data());
			settings.pauseModelPath = QString::fromUtf8(params["pauseModelPath"].get_str().data());
			settings.tresholdParam = params["tresholdParam"].get_real();

			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}


bool SpeakersCountPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
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
	
	try 
	{
		//////////////////////////////////////////////////////////////////////////////
		Settings settings;
		if ( !GetSettings(inData.PluginSettings, settings, lastErrorDesc) )
			return false;

		//загрузка моделей.
		eGmmTk gmmTk;
		SP_BYTE* UBM = NULL;
		if ( gmmTk.ReadModelFromFile(settings.ubmModelPath.toLocal8Bit().data(), UBM) != 0 )
		{
			lastErrorDesc = "Cannot load ubm model";
			return false;
		}

		SP_BYTE* speechModel = NULL;
		if ( gmmTk.ReadModelFromFile(settings.speechModelPath.toLocal8Bit().data(), speechModel) != 0 )
		{
			lastErrorDesc = "Cannot load speech model";
			gmmTk.DisposeMemory(UBM);
			return false;
		}

		SP_BYTE* nonSpeechModel = NULL;
		if ( gmmTk.ReadModelFromFile(settings.pauseModelPath.toLocal8Bit().data(), nonSpeechModel) )
		{
			lastErrorDesc = "Cannot load pause model";
			gmmTk.DisposeMemory(UBM);
			gmmTk.DisposeMemory(speechModel);
			return false;
		}


		outData.Marking.clear();
		int commonSize = 0;


		for ( QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.constBegin();
			iter != inData.Intervals.constEnd(); iter++ )
			commonSize += iter->second - iter->first;

		vector<int> sizeSlices(inData.Intervals.size());
		std::vector<short> data(commonSize);

		int offset = 0;
		int i = 0;

		for ( QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.constBegin();
			iter != inData.Intervals.constEnd(); iter++ )
		{
			memcpy(&data[offset], &((short *)inData.Signal.data())[iter->first], (iter->second - iter->first) * 2);
			offset += iter->second - iter->first;
			sizeSlices[i] = iter->second - iter->first;
			i++;
		}
				
		SDiSeCEngine sDiSeC;
		int outVal = inData.Intervals.size();
		vector<int> outClass(inData.Intervals.size());
		
		int res = sDiSeC.HowManySpeakers(&data[0], 
								&sizeSlices[0], 
								sizeSlices.size(),
								UBM, 
								speechModel, 
								nonSpeechModel, 
								settings.tresholdParam,
								outVal, 
								&outClass[0]);

		gmmTk.DisposeMemory(UBM);
		gmmTk.DisposeMemory(speechModel);
		gmmTk.DisposeMemory(nonSpeechModel);

		if ( res != 0 )
		{
			lastErrorDesc = QString("sDiSeC.HowManySpeakers error: %1").arg(res);
			return false;
		}
		else
		{
			int index = 0;
			for ( QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.constBegin();
				iter != inData.Intervals.constEnd(); iter++ )
			{
				outData.Marking.append( SUPPhraseData(iter->first, iter->second, 
							QString::number(outClass[index])));
				index++;
			}
			return true;
		}
	}
	catch(std::exception & e)
	{
		lastErrorDesc = QString::fromLocal8Bit("Ошибка исполнения: \"%1\".").arg(e.what());
	}
	catch(...)
	{
		lastErrorDesc = QString::fromLocal8Bit("Неизвестная ошибка.");
	}
	return false;
}



bool SpeakersCountPlugin::InitBatchProcessing(void * & internalData, int id, const QString & settingsStr, QString & errorDesc)
{
	return false;
	//ThreadData & threadData = *((ThreadData *)(internalData = new ThreadData));

	//if ( !GetSettings(settingsStr, threadData.settings, lastErrorDesc) )
	//	return false;

	////загрузка моделей.
	//if ( threadData.gmmTk.ReadModelFromFile(threadData.settings.ubmModelPath.toLocal8Bit().data(), 
	//						threadData.UBM) != 0 )
	//{
	//	lastErrorDesc = "Cannot load ubm model";
	//	return false;
	//}

	//if ( threadData.gmmTk.ReadModelFromFile(threadData.settings.speechModelPath.toLocal8Bit().data(), 
	//						threadData.speechModel) != 0 )
	//{
	//	lastErrorDesc = "Cannot load speech model";
	//	threadData.gmmTk.DisposeMemory(threadData.UBM);
	//	return false;
	//}

	//if ( threadData.gmmTk.ReadModelFromFile(threadData.settings.pauseModelPath.toLocal8Bit().data(), 
	//						threadData.nonSpeechModel) )
	//{
	//	lastErrorDesc = "Cannot load pause model";
	//	threadData.gmmTk.DisposeMemory(threadData.UBM);
	//	threadData.gmmTk.DisposeMemory(threadData.speechModel);
	//	return false;
	//}
	//return true;
}

bool SpeakersCountPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & file_report, QString & lastErrorDesc)
{
	return false;
	//QByteArray sound;
	//if ( !LoadFile(file_report.first, 16, 8000, sound, lastErrorDesc) )
	//	return false;

	//ThreadData & threadData = *((ThreadData *)internalData);

	//std::list< std::pair< std::string, std::pair<int,int>>> marks;
	//std::string error;

	//QList<SUPPhraseData> marking;

	//if( VAD_GMM::DoVad(threadData.vadModel,
	//									(const unsigned char*)sound.data(),
	//									sound.size(),			marks,
	//		error) )
	//{
	//	for ( std::list< std::pair< std::string, std::pair<int,int>>>::iterator mIter = marks.begin(); 
	//		mIter != marks.end(); mIter++ )
	//			marking.append( SUPPhraseData(mIter->second.first,
	//											mIter->second.second,
	//											QString::fromStdString(mIter->first)));

	//}
	//else
	//{
	//	lastErrorDesc = "VAD_GMM::DoVad error!!!";
	//}

	//try
	//{
	//	if ( threadData.settings.CutNoice )
	//	{
	//		QList<SUPPhraseData>::iterator iter = marking.begin();

	//		//Вырезаем шум.
	//		while ( iter != marking.end() )
	//		{
	//			//Вырезаем фразу из файла, из разметки и смещаем все остальные фразы на длинну данной.
	//			if ( (iter->Name == "noise") || (iter->Name.size() >= 2 &&  iter->Name[0] == '_' && iter->Name[1] == '_') )
	//			{
	//				//! Удалим звук.
	//				long len = iter->End - iter->Start;
	//				sound.remove(iter->Start * 2, len * 2);
	//				iter = marking.erase(iter);

	//				//! Сместим. разметку. 
	//				for ( QList<SUPPhraseData>::iterator curIter = iter; 
	//					curIter != marking.end(); curIter++ )
	//				{
	//					curIter->Start = curIter->Start - len;
	//					curIter->End = curIter->End - len;
	//				}
	//			}
	//			else 
	//				iter++;
	//		}

	//		//! Сохраним новый файл.
	//		if ( !SaveFile(threadData.settings.ModiffedFilesDir + "/" + QFileInfo(file_report.first).fileName(), 1, 16, 8000, sound, lastErrorDesc) )
	//			return false;
	//	}

	//	if ( SaveMarkingInFile(file_report.second, marking, lastErrorDesc) )
	//		return true;
	//	else
	//		return false;		
	//}
	//catch(std::exception & e)
	//{
	//	lastErrorDesc = QString::fromLocal8Bit("Ошибка при разборе результата JSON формата: \"%1\"").arg(e.what());
	//	return false;
	//}
	//catch(...)
	//{
	//	lastErrorDesc = QString::fromLocal8Bit("Неизвестная ошибка при разборе результата JSON формата");
	//	return false;
	//}
}

void SpeakersCountPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		delete (ThreadData *)internalData;
		internalData = NULL;
	}
}

Q_EXPORT_PLUGIN2(SpeakersCountPlugin, SpeakersCountPlugin);