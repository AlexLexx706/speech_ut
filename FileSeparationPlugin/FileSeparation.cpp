#include "FileSeparation.h"
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


using namespace std;

FileSeparationPlugin::FileSeparationPlugin():dialog(NULL),menu(NULL),isInit(false)
{}

FileSeparationPlugin::~FileSeparationPlugin()
{
	Free();
}

void FileSeparationPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_FileSeparation_Plugin");
	actions[1]->setObjectName("action_start_FileSeparation_batch_processing");
	actions[2]->setObjectName("action_stop_FileSeparation_batch_processing");
}

bool FileSeparationPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
{
	if ( !IsInit() )
	{
		if ( inSetting )
		{
			dialog = new SettingsDialog(*inSetting);
			menu = InitActions(inMenu, inToolBar, 0x19);
		}
		isInit = true;
		return true;
	}
	lastErrorDesc = QString::fromLocal8Bit("Плагин уже инициализирован.");
	return false;
}

bool FileSeparationPlugin::Free()
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

QString FileSeparationPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;
	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));
	parameters.push_back(json_spirit::Pair("outDir", dialog->GetOutDir()));
	parameters.push_back(json_spirit::Pair("markingAsUtf8", dialog->MarkingAsUtf8()));

	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool FileSeparationPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();
			settings.outDir = QString::fromUtf8(params["outDir"].get_str().data());
			settings.markingAsUtf8 = params["markingAsUtf8"].get_bool();
			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}

bool FileSeparationPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
{
	return false;
}

bool FileSeparationPlugin::PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc)
{
	return false;
}


bool FileSeparationPlugin::InitBatchProcessing(void * & internalData, int id, const QString & settingsStr, QString & errorDesc)
{
	BatchThreadData & threadData  = *((BatchThreadData *)(internalData = new BatchThreadData));

	if ( !GetSettings(settingsStr, threadData.settings, errorDesc) )
		return false;

	//Создадим выходную директорию
	QDir dir(threadData.settings.outDir);

	if ( !dir.exists() && !QDir().mkpath(threadData.settings.outDir) )
	{
		errorDesc = QString::fromLocal8Bit("Неудалось создать выходную директорию:\"%1\"").arg(threadData.settings.outDir);
		return false;
	}
	return true;
}

bool FileSeparationPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & lastErrorDesc)
{
	BatchThreadData & threadData  = *((BatchThreadData *)internalData);
	try 
	{
		//грузим файл.
		QByteArray sound;
		if ( !LoadFile(fileDesc.first, 16, 8000, sound, lastErrorDesc) )
			return false;

		//грузим разметки.
		QList<QPair<QString, QPair<long, long> > > marking;
		if ( !LoadMarking(fileDesc.second, threadData.settings.markingAsUtf8, marking, lastErrorDesc) )
			return false;

		//режим файл по разметки

		int counter = 1;
		char ssss[100];

		for ( QList<QPair<QString, QPair<long, long> > >::iterator iter = marking.begin(); 
			iter != marking.end(); iter++ )
		{
			//qDebug() << iter->second.first << iter->second.second - iter->second.first;
			QByteArray curSound = sound.mid(iter->second.first * 2, (iter->second.second - iter->second.first)*2);

			QString fileName = QString("%1\\%2_%3").arg(threadData.settings.outDir).
				arg(QFileInfo(fileDesc.first).completeBaseName()).arg(counter);

			QFile file(fileName+".wav");
			SoundRecorder::WriteWaveFile(&file, 1, 8000, 16, curSound);

			QFile labFile(fileName+".txt");
			labFile.open(QIODevice::WriteOnly | QIODevice::Text);
			if ( threadData.settings.markingAsUtf8 )
				labFile.write(iter->first.toUtf8().data());
			else
				labFile.write(iter->first.toLocal8Bit().data());
			counter++;
		}
		return true;
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

void FileSeparationPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		delete (BatchThreadData *)internalData;
		internalData = NULL;
	}
}

bool FileSeparationPlugin::LoadMarking(const QString & fileName,
									   bool markingAsUtf,
									   QList<QPair<QString, QPair<long, long> > > & data,
									   QString & lastErrorDesc)
{
	QFileInfo info(fileName);
	if ( info.suffix().toLower() == "lab" )
	{
		QFile file(fileName);
		if ( file.open(QFile::ReadOnly) )
		{
			QString sdp;
			if ( markingAsUtf )
				sdp = QString::fromUtf8(file.readAll());
			else
				sdp = QString::fromLocal8Bit(file.readAll());

			QStringList lines = sdp.split(QRegExp("(\\n|\\r|\\r\\n)"));
			QRegExp line("(\\d+)\\s+(\\d+) (.+)");

			for (QStringList::iterator iter = lines.begin(); 
				iter != lines.end(); iter++ )
			{
				if ( line.indexIn(*iter) > -1 )
				{

					long start = ((qreal)line.cap(1).toULongLong()) * 0.0000001 * 8000;
					long stop = ((qreal)line.cap(2).toULongLong()) * 0.0000001 * 8000;
					QString name = line.cap(3);
					if ( stop < start )
					{
						lastErrorDesc = QString::fromUtf8("Неправильная разметка stop < start: %1 < %2, файл: \"%3\"").
							arg(start).arg(stop).arg(fileName);
						return false;
					}
					data.append(QPair<QString, QPair<long, long> >(name, QPair<long, long>(start, stop)));
				}
			}
			if ( data.size() > 0)
				return true;
			lastErrorDesc = QString::fromUtf8("Пустая разметка файл: \"%1\"").arg(fileName);
			return false;
		}
	}
	return false;
}





Q_EXPORT_PLUGIN2(FileSeparationPlugin, FileSeparationPlugin);