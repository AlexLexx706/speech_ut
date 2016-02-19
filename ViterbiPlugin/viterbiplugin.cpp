
#include "viterbiplugin.h"
#include <list>
#include <iostream>
#include <string>
#include <QMenu>
#include <QToolBar>
#include <QtPlugin>
#include <QDir>

using namespace std;


ViterbiPlugin::ViterbiPlugin():BaseSUPlugin(),menu(NULL), dialog(NULL), isInit(false)
{
	lastErrorDesc = QString::fromLocal8Bit("Инициализация не производилась.");
}

ViterbiPlugin::~ViterbiPlugin()
{
	Free();
}

bool ViterbiPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
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

bool ViterbiPlugin::Free()
{
	if ( IsInit() )
	{
		delete dialog;
		dialog = NULL;
		delete menu;
		menu = NULL;
		isInit  = false;

		return true;
	}
	lastErrorDesc = QString::fromLocal8Bit("Ресурсы плагина уже освобождены.");
	return false;
}



QString ViterbiPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;

	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));


	parameters.push_back(json_spirit::Pair("modelFileName", dialog->GetModelFileName()));
	parameters.push_back(json_spirit::Pair("pruningType", (int)dialog->GetPruningType()));
	parameters.push_back(json_spirit::Pair("pruningPar", dialog->GetPruningPar()));
	parameters.push_back(json_spirit::Pair("loopActionsType", (int)dialog->GetLoopActionsType()));
	parameters.push_back(json_spirit::Pair("useLangModel", dialog->IsUseLangModel()));
	parameters.push_back(json_spirit::Pair("nGramSize", dialog->GetNGramSize()));


	parameters.push_back(json_spirit::Pair("nBestType", (int)dialog->GetNBestType()));
	parameters.push_back(json_spirit::Pair("nBestCount", dialog->GetNBestCount()));
	parameters.push_back(json_spirit::Pair("NBestCapacity", dialog->GetNBestCapacity()));
	parameters.push_back(json_spirit::Pair("printLatticeOutput", dialog->IsPrintLatticeOutput()));
	parameters.push_back(json_spirit::Pair("latticeOutputFile", dialog->GetLatticeOutputFile()));
	parameters.push_back(json_spirit::Pair("useLatticeOutputDir", dialog->IsUseLatticeOutputDir()));
	parameters.push_back(json_spirit::Pair("latticeOutputDir", dialog->GetLatticeOutputDir()));

	parameters.push_back(json_spirit::Pair("plp", dialog->get_plp_json_string_settings()));

	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));


	//json_spirit::Array ar;
	//json_spirit::Value v1(12);
	//json_spirit::Value v2("123121");
	//ar.push_back(v1);
	//ar.push_back(v2);
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool ViterbiPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.ModelFileName =		QString::fromUtf8(params["modelFileName"].get_str().data());
			settings.PruningType =			(PRUNING_T)params["pruningType"].get_int();
			settings.PruningPar =			params["pruningPar"].get_real();
			settings.LoopActionsType =		(LOOPACTIONS_T)params["loopActionsType"].get_int();
			settings.UseLangModel =			params["useLangModel"].get_bool();
			settings.NGramSize =			params["nGramSize"].get_int();

			settings.NBestType =			(NBEST_T)params["nBestType"].get_int();
			settings.NBestCount =			params["nBestCount"].get_int();
			settings.NBestCapacity =		params["NBestCapacity"].get_int();
			settings.PrintLatticeOutput=	params["printLatticeOutput"].get_bool();
			settings.LatticeOutputFile =	QString::fromUtf8(params["latticeOutputFile"].get_str().data());
			settings.UseLatticeOutputDir =	params["useLatticeOutputDir"].get_bool();
			settings.LatticeOutputDir =		QString::fromUtf8(params["latticeOutputDir"].get_str().data());
			
			json_spirit::mValue v;
			json_spirit::read(params["plp"].get_str(), v);
			settings.plp = v.get_obj();
			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}


bool ViterbiPlugin::ComputePhonemes(MoreEngine & recEngine, 
									 const std::vector<short>  & signal, 
									 const Settings & settings,
									 const char * fileName,
									 std::vector<std::list<std::pair<int, std::string>>>& outputPath,
									 QString & lastErrorDesc)
{
	//1. Препроцессинг.
	std::vector<std::vector<SP_REAL>> featureMap;
	//int error_code = ppc.PreProcessing(signal, featureMap, PLP, ZEROMEAN, DELTADELTA, 0);

	json_spirit::mObject::const_iterator iter;
	std::vector<SP_REAL> setup_params;

	if ( (iter = settings.plp.find("setup_params")) != settings.plp.end())
	{
		const json_spirit::mArray & arr = iter->second.get_array();
		for (int i = 0; i < arr.size(); i++ )
			setup_params.push_back(arr[i].get_real());
	}

	std::pair<bool, VTLN_struct> vtln(false,VTLN_struct());

	if ( (iter = settings.plp.find("vtln_struct")) != settings.plp.end())
	{
		vtln.first = true;
		const json_spirit::mObject & obj = iter->second.get_obj();
		vtln.second.alpha = obj.find("alpha")->second.get_real();
		vtln.second.f_l = obj.find("f_l")->second.get_real();
		vtln.second.f_h = obj.find("f_h")->second.get_real();
		vtln.second.f_max = obj.find("f_max")->second.get_real();
		vtln.second.f_min = obj.find("f_min")->second.get_real();
	}

	int error_code = ppc.PreProcessing(signal,
			featureMap,
			(FEATURE_T)settings.plp.find("features_type")->second.get_int(),
			(NORMALIZE_T)settings.plp.find("normalize_type")->second.get_int(),
			(DELTA_T)settings.plp.find("delta_type")->second.get_int(),
			setup_params.empty() ? NULL : &setup_params[0],
			vtln.first ? &vtln.second : NULL);

	
	if (error_code  < 0)
	{
		lastErrorDesc = QString("PreProcessing failed. error code %1").arg(error_code);
		return false;
	}

	SP_REAL outputProb = 0;

	//! Получение фонем.
	error_code = recEngine.VDecode(featureMap, 
									outputPath, 
									outputProb,
									settings.PruningType,
									settings.PruningPar,
									settings.LoopActionsType,
									settings.UseLangModel,
									settings.NGramSize, 
									settings.NBestType,
									settings.NBestCount,
									settings.NBestCapacity,
									fileName,
									fileName);
	recEngine.VReset();
	if (error_code < 0)
	{
		lastErrorDesc = QString("VDecode failed. error code %1").arg(error_code);
	}
	return true;
}


void ViterbiPlugin::AddMarking(int sampleRate, 
							   long offset, 
							   const std::vector<std::list<std::pair<int, std::string>>> & __results,
							   QList<SUPPhraseData> & marking)
{
	if ( __results.size() )
	{
		const std::list<std::pair<int, std::string>> & results = __results[0];

		const qreal winShift = (100000. * 0.0000001)/(1./(qreal)sampleRate); // 10 mSec
		std::list<pair<int, string>>::const_iterator startIter = results.begin();
		
		for ( list<pair<int, string>>::const_iterator iter = startIter;
			iter != results.end(); iter++ )
		{
			int start = iter == startIter ? 0 : startIter->first;
			int end = iter->first;

			marking.append(SUPPhraseData(offset + winShift * start, offset + winShift * end, QString::fromStdString(iter->second)));
			startIter = iter;
		}
	}
}


bool ViterbiPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
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
		Settings settings;
		if ( !GetSettings(inData.PluginSettings, settings, lastErrorDesc) )
			return false;

		MoreEngine recEngine;

		//1. Инициализация.
		int error_code = recEngine.VBegin();
		if (error_code < 0)
		{
			lastErrorDesc = QString("VBegin error code: %1").arg(error_code);
			return false;
		}

		//2. загрузка модеделей.
		QByteArray modelName = settings.ModelFileName.toLocal8Bit();
		char * data[1] = {modelName.data()};

		if( (error_code = recEngine.VLoadPhones(HTK_PHONEME, (const char**)data))  < 0 )
		{
			lastErrorDesc = QString("model cannot be loaded: %1. error code %2").arg(settings.ModelFileName).arg(error_code);
			return false;
		}

		outData.Marking.clear();

		std::vector<short> signal;
		int count = 1;
		bool res = false;

		for ( QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.begin(); 
			iter != inData.Intervals.end(); iter++ )
		{
			std::vector<std::list<std::pair<int, std::string>>> results;
			signal.resize(iter->second - iter->first);
			memcpy(&signal[0], &((short *)inData.Signal.data())[iter->first], (iter->second - iter->first) * 2);
			QString strError;

			if ( ComputePhonemes(recEngine, 
							signal, 
							settings, 
							(settings.PrintLatticeOutput ? settings.LatticeOutputFile.toLocal8Bit().data() : NULL),
							results,
							strError) )
			{
				AddMarking(inData.SampleRate, iter->first, results, outData.Marking);
				res = true;
			}
			else 
				emit ProgressMessage(id, count/(qreal)inData.Intervals.size(), 
						QString::fromLocal8Bit("Frame(%1 %2)Неудалось рассчитать фонемы для фрейма, ошибка:").arg(iter->first).
						arg(iter->second).
						arg(strError));
			count++;
		}

		lastErrorDesc = res ? "" : QString::fromLocal8Bit("Неудалось рассчитать не одной фонемы!");
		return res;
	
	}
	catch(std::exception & e)
	{
		lastErrorDesc = QString::fromLocal8Bit("Unknown error:").arg(e.what());
	}
	catch(...)
	{
		lastErrorDesc = QString::fromLocal8Bit("Unknown error");
	}
	return false;
}

bool ViterbiPlugin::InitBatchProcessing(void * & internalData, int id,  const QString & settingsStr, QString & errorDesc)
{
	internalData = new BatchThreadData;
	BatchThreadData & threadData = *((BatchThreadData *)internalData);

	if ( !GetSettings(settingsStr, threadData.settings, errorDesc) )
	{
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}

	if ( threadData.settings.UseLatticeOutputDir &&  !QDir(threadData.settings.LatticeOutputDir).exists() )
	{
		errorDesc = QString::fromLocal8Bit("Директория:\"%1\" для сохранения латисов не существует").arg(threadData.settings.LatticeOutputDir);
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}

	int error_code = threadData.batchRecEngine.VBegin();

	if (error_code < 0)
	{
		errorDesc = QString("VBegin error code: %1").arg(error_code);
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}

	//2. загрузка модеделей.
	QByteArray modelName = threadData.settings.ModelFileName.toLocal8Bit();
	char * data[1] = {modelName.data()};

	if( (error_code = threadData.batchRecEngine.VLoadPhones(HTK_PHONEME, (const char**)data))  < 0 )
	{
		lastErrorDesc = QString("model cannot be loaded: %1. error code %2").arg(threadData.settings.ModelFileName).arg(error_code);
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}

	//! Инициализация.
	return true;
}

bool ViterbiPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & file_report, QString & errorDesc)
{
	BatchThreadData & threadData = *((BatchThreadData *)internalData);
	
	QByteArray * buffer = new QByteArray();

	if ( !LoadFile(file_report.first, 16, 8000, *buffer, errorDesc) )
	{
		delete buffer;
		return false;
	}
	std::vector<short> signal(buffer->size()/2);
	memcpy(&signal[0], buffer->data(), buffer->size());
	delete buffer;

	//Имя флайла латиса.
	QFileInfo info(file_report.first);
	QString latFilePath = info.fileName();
	int sSize = info.suffix().size()+1;
	latFilePath.remove(latFilePath.size()  - sSize, sSize);
	latFilePath = threadData.settings.LatticeOutputDir + "/" + latFilePath + ".lat";

	std::vector<std::list<std::pair<int, std::string>>> results;

	if ( ComputePhonemes(threadData.batchRecEngine, 
					signal, 
					threadData.settings, 
					(threadData.settings.UseLatticeOutputDir ? latFilePath.toLocal8Bit().data() : NULL),
					results,
					errorDesc) )
	{
		QList<SUPPhraseData> marking;
		AddMarking(8000, 0, results, marking);
		return SaveMarkingInFile(file_report.second, marking, errorDesc);
	}
	return false;
}

void ViterbiPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		delete ((BatchThreadData *)internalData);
		internalData = NULL;
	}
}

void ViterbiPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_Viterbi_Plugin");
	actions[1]->setObjectName("action_get_Viterbi_marking");
	actions[2]->setObjectName("action_get_Viterbi_marking_on_marking");
	actions[3]->setObjectName("action_start_Viterbi_batch_processing");
	actions[4]->setObjectName("action_stop_Viterbi_batch_processing");
}


Q_EXPORT_PLUGIN2(ViterbiPlugin, ViterbiPlugin);