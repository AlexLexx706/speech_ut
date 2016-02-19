#include "kwfinderplugin.h"
#include <QFileInfo>
#include <QString>
#include <QDir>
#include <iostream>
#include <conio.h>
#include <string>
#include "../json_spirit/json_spirit.h"
#include <Auxiliary/libsnd/include/sndfile.h>
#include <QMenu>
#include <QToolBar>
#include <QtPlugin>
#include <boost/lambda/lambda.hpp> 
#include <boost/lambda/bind.hpp>
#include <Preprocessing/ppcEngine.h>


#pragma warning(disable : 4267)


KWFinderPlugin::KWFinderPlugin():dialog(NULL),menu(NULL), isInit(false)
{}

KWFinderPlugin::~KWFinderPlugin()
{
	Free();
}

void KWFinderPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_KWFinder_Plugin");
	actions[1]->setObjectName("action_get_KWFinder_marking");
	actions[2]->setObjectName("action_get_KWFinder_marking_on_marking");
	actions[3]->setObjectName("action_start_KWFinder_batch_processing");
	actions[4]->setObjectName("action_stop_KWFinder_batch_processing");
}

bool KWFinderPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
{
	if ( !IsInit())
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

bool KWFinderPlugin::Free()
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
	else 
		lastErrorDesc = QString::fromLocal8Bit("Ресурсы уже освобождены.");	
	return false;
}





QString KWFinderPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;
	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));

	parameters.push_back(json_spirit::Pair("pathToHmm", dialog->GetPathToHmm()));
	parameters.push_back(json_spirit::Pair("pathToDict", dialog->GetPathToDict()));
	parameters.push_back(json_spirit::Pair("dictInUtf8", dialog->IsDictInUtf8()));
	parameters.push_back(json_spirit::Pair("pathToTiedls", dialog->GetPathToTiedls()));

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


	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool KWFinderPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.PathToHmm = QString::fromUtf8(params["pathToHmm"].get_str().data());
			settings.PathToDict = QString::fromUtf8(params["pathToDict"].get_str().data());
			settings.dictInUtf8 = params["dictInUtf8"].get_bool();
			settings.PathToTiedls = QString::fromUtf8(params["pathToTiedls"].get_str().data());

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

			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}


bool KWFinderPlugin::FindWords(MoreEngine & engine,
								const Settings & __settings,
								const char * latticeFileName,
								  const short* signal, 
								  int signalSize, 
								  std::vector<std::list<std::pair<int, std::string> > > & outputPath,
								  QString & errorDesc)
{
	using namespace boost::lambda;
	
	std::string output;
	std::vector<std::vector<SP_REAL>> featureMap;
	std::vector<short> input(signal, signal + signalSize);

	SP_REAL settings[] = {8000.0, 0.025, 0.01, 26.0, 13.0, 1.0, 22.0, 0.97, 0.33, 0, 4000};

	int result = -1;
	ppcEngine ppc;

	if( (result = ppc.PreProcessing(input, featureMap, PLP, ZEROMEAN, DELTADELTA, 0)) < 0)
	{
		errorDesc = QString::fromLocal8Bit("PreProcessing failed, error:").arg(result);
		return false;
	}

	SP_REAL outputProb;

	if( (result = engine.VDecode(featureMap, 
								outputPath, 
								outputProb, 
								__settings.PruningType, 
								__settings.PruningPar,
								__settings.LoopActionsType,
								__settings.UseLangModel,
								__settings.NGramSize,
								__settings.NBestType,
								__settings.NBestCount,
								__settings.NBestCapacity,
								(bool)latticeFileName,
								latticeFileName)) < 0 )
	{
		errorDesc = QString::fromLocal8Bit("VDecode failed").arg(result);
		engine.VReset();
		return false;
	}
	else
	{
		engine.VReset();

		if ( __settings.NBestCount )
		{
			int scale = static_cast<int>(settings[2] * 10000);

			for (std::list<std::pair<int, std::string> >::iterator iter = outputPath[0].begin();
				iter != outputPath[0].end(); iter++ )
				iter->first *= scale;

			return outputPath.size();
		}
		else 
		{
			return true;
		}
	}
}




bool KWFinderPlugin::AddMarking(int sampleRate, 
								int offset, 
								bool useUtf8,
								const std::vector<std::list<std::pair<int, std::string> > > & _result,
								QList<SUPPhraseData> & marking, 
								QString & lastErrorDesc)
{
	if ( _result.size() )
	{
		const std::list<std::pair<int, std::string> > & result = _result[0];
		const qreal winShift = 1e-2 * sampleRate;
		qint32 start = offset;
		qint32 end = start;

		for ( std::list<std::pair<int, std::string> >::const_iterator iter = result.begin(); 
			iter != result.end(); iter++ )
		{
			end = iter->first * winShift + offset;
			marking.append(SUPPhraseData(start, end, 
				!useUtf8 ? QString::fromLocal8Bit(iter->second.data()) : QString::fromUtf8(iter->second.data())  ));
			start = end;
		}
		return true;

	}
	else 
		lastErrorDesc = QString("Empty result");
	return false;
}

bool KWFinderPlugin::InitEngine(MoreEngine & engine, const Settings & settings, 
								QString & lastErrorDesc)
{
	if(!QFileInfo(settings.PathToHmm).exists())
	{
		lastErrorDesc = "hmmdefs weren't found";
		return false;
	}

	if(!QFileInfo(settings.PathToDict).exists())
	{
		lastErrorDesc = "dict wasn't found";
		return false;
	}

	if(!QFileInfo(settings.PathToTiedls).exists())
	{
		lastErrorDesc = "tiedls weren't found";
		return false;
	}

	int error_code = engine.VBegin();

	if (error_code < 0)
	{
		lastErrorDesc = QString("VBegin error code: %1").arg(error_code);
		return false;
	}


	//2. загрузка модеделей.
	QByteArray hmm = settings.PathToHmm.toLocal8Bit();
	QByteArray dict = settings.PathToDict.toLocal8Bit();
	QByteArray tields = settings.PathToTiedls.toLocal8Bit();

	const char* temp[3] = {hmm.data(), tields.data(), dict.data()};

	if( (error_code = engine.VLoadPhones(HTK_WORDS,temp))  < 0 )
	{
		lastErrorDesc = QString("model cannot be loaded. error code %1").arg(error_code);
		return false;
	}

	//! Инициализация.
	return true;
}





bool KWFinderPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
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

		MoreEngine engine;
		if ( !InitEngine(engine, settings, lastErrorDesc) )
			return false;

		//Поиск.
		bool res = false;
		outData.Marking.clear();

		for (QList< QPair<qint32, qint32> >::const_iterator iter = inData.Intervals.constBegin(); 
			iter != inData.Intervals.constEnd(); iter++ )
		{
			std::vector<std::list<std::pair<int, std::string> > > result;
			QString errorDesc;

			if ( !FindWords(engine, 
					settings,
					settings.PrintLatticeOutput ? settings.LatticeOutputFile.toLocal8Bit().data() : NULL,
					&((short *)inData.Signal.data())[iter->first], 
					iter->second - iter->first, 
					result, 
					errorDesc) )
			{
				emit ProgressMessage(id, 0, QString::fromLocal8Bit("Frame(%1, %2) kw_find returned error:%1").
						arg(iter->first).arg(iter->second).arg(errorDesc));
			}
			else
			{
				if ( !AddMarking(inData.SampleRate,
								iter->first,
								settings.dictInUtf8,
								result,
								outData.Marking,
								lastErrorDesc) )
					emit ProgressMessage(id, 0, QString::fromLocal8Bit("Frame(%1, %2) Неудалось создать разметку").arg(iter->first));
				else if ( !res )
					res = true;
			}
		}
	
		lastErrorDesc = res ? "" : QString::fromLocal8Bit("Неудалось найти не одного слова.");
		return res;

	
	}
	catch(...)
	{
		lastErrorDesc = QString("Unknown error: ");
	}
	return false;
}




bool KWFinderPlugin::InitBatchProcessing(void * & internalData, int id,  const QString & settingsStr, QString & lastErrorDesc)
{
	internalData = new BatchThreadData;
	Settings & settings = ((BatchThreadData*)internalData)->settings;

	if ( !GetSettings(settingsStr, settings, lastErrorDesc) )
	{
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}

	if ( settings.UseLatticeOutputDir && !QDir(settings.LatticeOutputDir).exists() )
	{
		lastErrorDesc = QString::fromLocal8Bit("Директория: \"%1\" для сохранения латтисов не существует").arg(settings.LatticeOutputDir);
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}



	((BatchThreadData *)internalData)->batchRecEngine = new MoreEngine;
	if( !InitEngine(*(((BatchThreadData *)internalData)->batchRecEngine), 
				((BatchThreadData*)internalData)->settings, lastErrorDesc) )
	{
		delete ((BatchThreadData *)internalData)->batchRecEngine;
		delete (BatchThreadData *)internalData;
		internalData = NULL;
		return false;
	}
	return true;
}


bool KWFinderPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc)
{
	//static int __count = 0;
	//if (__count > 3)
	//{
	//	int * ss = (int * )0;
	//	*ss = 1212;
	//}
	//__count++;

	QByteArray signal;
	const int sampleRate = 8000;
	const int bitPerSample = 16;

	if ( !LoadFile(fileDesc.first, bitPerSample, sampleRate, signal, errorDesc) )
		return false;

	std::vector<std::list<std::pair<int, std::string> > > result;
	BatchThreadData & data = *((BatchThreadData *)internalData);
	
	QFileInfo info(fileDesc.first);
	QString latFilePath = info.fileName();
	int sSize = info.suffix().size()+1;
	latFilePath.remove(latFilePath.size()  - sSize, sSize);
	latFilePath = data.settings.LatticeOutputDir + "/" + latFilePath + ".lat";

	if ( FindWords(*data.batchRecEngine, 
					data.settings,
					(data.settings.UseLatticeOutputDir ? latFilePath.toLocal8Bit().data() : NULL),
					(const short *)signal.data(), 
					signal.size()/2,
					result, 
					errorDesc) )
	{
		if ( data.settings.NBestCount )
		{
			QList<SUPPhraseData> marking;
			if ( AddMarking(sampleRate,
							0,
							data.settings.dictInUtf8,
							result,
							marking,
							errorDesc) )
				return SaveMarkingInFile(fileDesc.second, marking, errorDesc);
		}
		else 
			return true;
	}
	return false;
}

void KWFinderPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		delete ((BatchThreadData *)internalData)->batchRecEngine;
		delete (BatchThreadData *)internalData;
		internalData = NULL;
	}
}


Q_EXPORT_PLUGIN2(kwFinderPlugin, KWFinderPlugin);