#include "keywordspottingplugin.h"
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
#include <QUuid>
#include <QTime>
#include <boost/lambda/lambda.hpp> 
#include <boost/lambda/bind.hpp>
#include <Preprocessing/ppcEngine.h>

#pragma warning(disable : 4267)


KeywordSpottingPlugin::KeywordSpottingPlugin():dialog(NULL),menu(NULL),isInit(false)
{}

KeywordSpottingPlugin::~KeywordSpottingPlugin()
{
	Free();
}

void KeywordSpottingPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_KeywordSpotting_Plugin");
	actions[1]->setObjectName("action_get_KeywordSpotting_marking");
	actions[2]->setObjectName("action_get_KeywordSpotting_marking_on_marking");
	actions[3]->setObjectName("action_start_KeywordSpotting_batch_processing");
	actions[4]->setObjectName("action_stop_KeywordSpotting_batch_processing");
}


bool KeywordSpottingPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
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

bool KeywordSpottingPlugin::Free()
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


QString KeywordSpottingPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;
	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));


	parameters.push_back(json_spirit::Pair("pathToHmm", dialog->GetPathToHmm()));
	parameters.push_back(json_spirit::Pair("pathToDict", dialog->GetPathToDict()));
	parameters.push_back(json_spirit::Pair("dictInUtf8", dialog->IsDictInUtf()));


	parameters.push_back(json_spirit::Pair("pathToTiedls", dialog->GetPathToTiedls()));
	parameters.push_back(json_spirit::Pair("pathToKeywordsDict", dialog->GetPathToKeywordsDict()));

	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));

	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool KeywordSpottingPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
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
			settings.PathToKeywordsDict = QString::fromUtf8(params["pathToKeywordsDict"].get_str().data());
			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}



bool KeywordSpottingPlugin::AddMarking(int sampleRate, 
								int offset,
								bool dictInUtf8,
								 const std::vector<std::list<std::pair<int, std::string> > > & _result, 
								 const QStringList & keywords,
								 QList<SUPPhraseData> & marking, 
								 QString & lastErrorDesc)
{
	if ( _result.size() )
	{
		const std::list<std::pair<int, std::string> > & result = _result[0];
		const qreal winShift = 1e-3 * sampleRate;
		qint32 start = offset;
		qint32 end = start;

		bool res = false;

		for ( std::list<std::pair<int, std::string> >::const_iterator iter = result.begin(); 
			iter != result.end(); iter++ )
		{
			end = iter->first * winShift + offset;
			QString word = !dictInUtf8 ? QString::fromLocal8Bit(iter->second.data()) : QString::fromUtf8(iter->second.data());
			bool canAdd = false;

			for ( QStringList::const_iterator kIter = keywords.constBegin();
				kIter != keywords.end(); kIter++ )
			{
				if ( word == *kIter )
				{
					marking.append(SUPPhraseData(start, end, word));
					res = true;
					break;
				}
			}
			start = end;
		}
		lastErrorDesc = res ? "" : QString::fromLocal8Bit("Ключевые слова не найдены!");
		return res;
	}
	else 
		lastErrorDesc = QString("Empty result");
	return false;
}


bool KeywordSpottingPlugin::InitEngine(MoreEngine & engine, const Settings & settings, QString & commonDictFile, QString & lastErrorDesc)
{
	if(!QFileInfo(settings.PathToHmm).exists())
	{
		lastErrorDesc = "hmmdefs weren't found";
		return false;
	}

	if(!QFileInfo(settings.PathToTiedls).exists())
	{
		lastErrorDesc = "tiedls weren't found";
		return false;
	}

	if ( !QFileInfo(commonDictFile).exists() )
	{
		lastErrorDesc = QString::fromLocal8Bit("commonDict:\"%1\" не найден!").arg(commonDictFile);
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
	QByteArray dict = commonDictFile.toLocal8Bit();
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



bool KeywordSpottingPlugin::FindWords(MoreEngine & engine, 
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
								NO_PRUN,
								0,
								LOOP_RECORD,
								0,
								0) ) < 0 )
	{
		errorDesc = QString::fromLocal8Bit("VDecode failed").arg(result);
		engine.VReset();
		return false;
	}
	else
	{
		engine.VReset();
		int scale = static_cast<int>(settings[2] * 1000);
		for (std::list<std::pair<int, std::string> >::iterator iter =  outputPath[0].begin(); 
			iter != outputPath[0].end(); iter++ )
		{
			iter->first *= scale;
		}
	}

	return outputPath.size();
}


bool KeywordSpottingPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
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

		QString commonDictFile;
		QStringList keywords;
		if ( !CreateCommonDict(settings.PathToDict, settings.PathToKeywordsDict, settings.dictInUtf8,
			commonDictFile, keywords, lastErrorDesc) )
			return false;

		MoreEngine engine;
		if ( !InitEngine(engine, settings, commonDictFile, lastErrorDesc) )
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
				if ( !AddMarking(inData.SampleRate, iter->first, settings.dictInUtf8, result, keywords, outData.Marking, lastErrorDesc) )
					emit ProgressMessage(id, 0, QString::fromLocal8Bit("Frame(%1, %2) Неудалось создать разметку").arg(iter->first));
				else if ( !res )
					res = true;
			}
		}

		QFile::remove(commonDictFile);

		
		lastErrorDesc = res ? "" : QString::fromLocal8Bit("Неудалось найти не одного слова.");
		return res;

	
	}
	catch(...)
	{
		lastErrorDesc = QString("Unknown error: ");
	}

	return false;
}


bool KeywordSpottingPlugin::BeginBatchProcessing(int id, const QString & settingsStr, QString & errorDesc)
{
	Settings settings;
	if ( !GetSettings(settingsStr, settings, lastErrorDesc) )
		return false;

	if ( !CreateCommonDict(settings.PathToDict, settings.PathToKeywordsDict, settings.dictInUtf8,
				commonDictFile, keywords, lastErrorDesc) )
		return false;
	return true;
}

void KeywordSpottingPlugin::EndBatchProcessing(int id)
{
	QFile::remove(commonDictFile);
	commonDictFile = "";
	keywords.clear();
}

bool KeywordSpottingPlugin::InitBatchProcessing(void * & internalData, int id,  const QString & settingsStr,  QString & lastErrorDesc)
{
	Settings settings;
	if ( !GetSettings(settingsStr, settings, lastErrorDesc) )
		return false;

	MoreEngine * engine = new MoreEngine;
	if( !InitEngine(*engine, settings, commonDictFile, lastErrorDesc) )
	{
		delete engine;
		return false;
	}

	internalData = new BatchThreadData;
	((BatchThreadData *)internalData)->batchRecEngine = engine;
	((BatchThreadData *)internalData)->settings = settings;
	return true;
}

bool KeywordSpottingPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc)
{
	QByteArray signal;
	const int sampleRate = 8000;
	const int bitPerSample = 16;

	if ( !LoadFile(fileDesc.first, bitPerSample, sampleRate, signal, errorDesc) )
		return false;

	std::vector<std::list<std::pair<int, std::string> > > result;

	if ( FindWords(*((BatchThreadData *)internalData)->batchRecEngine, 
					(const short *)signal.data(), 
					signal.size()/2,
					result, 
					errorDesc) )
	{
		QList<SUPPhraseData> marking;
		if ( AddMarking(sampleRate, 
						0,
						((BatchThreadData *)internalData)->settings.dictInUtf8,
						result,
						keywords,
						marking,
						errorDesc) )
			return SaveMarkingInFile(fileDesc.second, marking, errorDesc);
	}
	return false;
}

void KeywordSpottingPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		delete ((BatchThreadData *)internalData)->batchRecEngine;
		delete (BatchThreadData *)internalData;
		internalData = NULL;
	}
}


bool KeywordSpottingPlugin::CreateCommonDict(const QString & dict, 
											 const QString & keywordsDict,
											 bool dictInUtf8,
											 QString & outCommonDict, 
											 QStringList & outKeywords, 
											 QString & outErrorDesc)
{
	//Грузим словарь
	QFile dictFile(dict);
	if ( dictFile.open(QIODevice::ReadOnly) )
	{
		QString __str = !dictInUtf8 ? QString::fromLocal8Bit(dictFile.readAll()) : 
					QString::fromUtf8(dictFile.readAll());

		QStringList dictLines = __str.split(QRegExp("(\\n|\\r|\\r\\n)"), QString::SkipEmptyParts);
		
		if ( dictLines.size() )
		{
			//! Грузим ключевые слова.
			QFile keywordsFile(keywordsDict);
			if ( keywordsFile.open(QIODevice::ReadOnly) )
			{
				__str = !dictInUtf8 ? QString::fromLocal8Bit(keywordsFile.readAll()) : 
							QString::fromUtf8(keywordsFile.readAll());

				QStringList keywordsLines = __str.split(QRegExp("(\\n|\\r|\\r\\n)"), QString::SkipEmptyParts);

				if ( keywordsLines.size() )
				{
					QList< QString * > commonList;
					QList< QStringList > kwTemp;
					outKeywords.clear();

					//! Добавим ключевые слова.
					for (QStringList::iterator kIter = keywordsLines.begin(); kIter != keywordsLines.end(); kIter++ )
					{
						QStringList checkList = kIter->split(QRegExp("\\s+"));
						outKeywords << checkList.takeFirst();
						kwTemp.push_back(checkList);
						commonList.append(&(*kIter));
					}


					//Создадим общий список.
					for (QStringList::iterator dIter = dictLines.begin(); dIter != dictLines.end(); dIter++ )
					{
						bool isSame = false;
						for (QList< QStringList >::iterator kIter = kwTemp.begin(); kIter != kwTemp.end(); kIter++ )
						{
							if ( IsWordsSimilarity(*dIter, *kIter) )
							{
								isSame = true;
								kwTemp.erase(kIter);
								break;
							}
						}

						if ( !isSame )
							commonList.push_back(&(*dIter));
					}

					//Создадим общий словарь
					outCommonDict = QUuid::createUuid().toString() + ".txt";

					QFile file(outCommonDict);
					if ( file.open(QFile::WriteOnly) )
					{
						QList< QString * >::iterator nextIter;
						for ( QList< QString * >::iterator iter = commonList.begin(); 
							iter != commonList.end(); iter++)
						{
							nextIter = iter;
							nextIter++;

							if ( nextIter != commonList.end() )
								file.write((*iter)->toLocal8Bit() + "\r\n");
							else 
								file.write((*iter)->toLocal8Bit());
						}
						file.close();
						outCommonDict = QFileInfo(outCommonDict).absoluteFilePath();
						return true;
					}
					else
						outErrorDesc = QString::fromLocal8Bit("Неудалось создать общий словарь: \"%1\" пустой.").arg(outCommonDict);
				}
				else 
					outErrorDesc = QString::fromLocal8Bit("Словарь ключевых слов: \"%1\" пустой.").arg(keywordsDict);

			}
			else 
				outErrorDesc = QString::fromLocal8Bit("Неудалось загрузить словарь ключевых слов: \"%1\"").arg(keywordsDict);
		}
		else 
			outErrorDesc = QString::fromLocal8Bit("Словарь: \"%1\" пустой.").arg(dict);

	}
	else 
		outErrorDesc = QString::fromLocal8Bit("Неудалось загрузить словарь: \"%1\"").arg(dict);
	return false;
}

bool KeywordSpottingPlugin::IsWordsSimilarity(const QString & w1, const QStringList & list)
{
	QStringList curList = w1.split(QRegExp("\\s+"));
	curList.pop_front();
	return curList == list;
}




Q_EXPORT_PLUGIN2(KeywordSpottingPlugin, KeywordSpottingPlugin);