#include "latticePlugin.h"
#include <iostream>
#include "../json_spirit/json_spirit.h"
#include <QMenu>
#include <QToolBar>
#include <QtPlugin>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QUuid>

using namespace std;


latticePlugin::latticePlugin():dialog(NULL),menu(NULL),isInit(false)
{}

latticePlugin::~latticePlugin()
{
	Free();
}

void latticePlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_lattice_Plugin");
	actions[1]->setObjectName("action_get_lattice_marking");
	actions[2]->setObjectName("action_get_lattice_marking_on_marking");
	actions[3]->setObjectName("action_start_lattice_batch_processing");
	actions[4]->setObjectName("action_stop_lattice_batch_processing");
}

bool latticePlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
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

bool latticePlugin::Free()
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

QString latticePlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;
	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));

	//! Сохраним ключи.
	json_spirit::Object keys;
	QMap<QString, QString> tParam = dialog->GetParameters();

	for ( QMap<QString, QString>::iterator iter = tParam.begin();
		iter != tParam.end(); iter++ )
		keys.push_back(json_spirit::Pair(iter.key().toUtf8().data(), iter->toUtf8().data()));

	parameters.push_back(json_spirit::Pair("keys", keys));
	parameters.push_back(json_spirit::Pair("programmFileName", dialog->GetProgrammFileName()));
	
	parameters.push_back(json_spirit::Pair("startThreshold", dialog->GetStartThreshold()));
	parameters.push_back(json_spirit::Pair("endThreshold", dialog->GetEndThreshold()));
	parameters.push_back(json_spirit::Pair("showBest", dialog->ShowBest()));
	parameters.push_back(json_spirit::Pair("latticeFile", dialog->GetLatticeFile()));
	parameters.push_back(json_spirit::Pair("usePostProcessing", dialog->UsePostProcessing()));



	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));
	return QString::fromUtf8(json_spirit::write(topObj,json_spirit::pretty_print | json_spirit::raw_utf8).data());
}

bool latticePlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toUtf8().data(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.programmFileName = QString::fromUtf8(params["programmFileName"].get_str().data());
			settings.ShowBest = params["showBest"].get_bool();
			settings.StartThreshold =  params["startThreshold"].get_int();
			settings.EndThreshold = params["endThreshold"].get_int();
			settings.UsePostProcessing = params["usePostProcessing"].get_bool();



			settings.latticeFile = QString::fromUtf8(params["latticeFile"].get_str().data());

			json_spirit::mObject & keys = params["keys"].get_obj();

			settings.keys.clear();
			for ( json_spirit::mObject::iterator iter = keys.begin(); 
				iter != keys.end(); iter++ )
				settings.keys[QString::fromUtf8(iter->first.data())] = QString::fromUtf8(iter->second.get_str().data());


			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}

QString GetProcessError( QProcess::ProcessError error)
{
	struct ErrorDesc
	{
		QProcess::ProcessError Error;
		const char * desc;
	};
	ErrorDesc map[] = 
	{
		{QProcess::FailedToStart,"FailedToStart"},
		{QProcess::Crashed,"Crashed"},
		{QProcess::Timedout,"Timedout"},
		{QProcess::ReadError,"ReadError"},
		{QProcess::WriteError,"WriteError"},
		{QProcess::UnknownError,"UnknownError"},
	};
	for ( int i = 0; i < sizeof(map)/sizeof(map[0]); i++ )
	{
		if ( map[i].Error == error )
			return map[i].desc;
	}
	return "Unknown errro";
}

bool latticePlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & lastErrorDesc)
{
	Settings settings;
	if ( !GetSettings(inData.PluginSettings, settings, lastErrorDesc) )
		return false;

	QProcess * process = new QProcess();
	QStringList args;
	QString path = QFileInfo(settings.programmFileName).absolutePath();

	//! Создадим свой лист файл.
	QString listFileName(QUuid::createUuid().toString() + ".list");
	QFile listFile(path + "/" + listFileName);

	if ( listFile.open(QIODevice::WriteOnly) )
	{
		listFile.write(settings.latticeFile.toLocal8Bit().data());
		listFile.close();
	}
	else 
	{
		lastErrorDesc = QString::fromLocal8Bit("Немогу создать файл со списком латисов: \"%1\"").arg(listFileName);
		return false;
	}

	settings.keys["--list-file"] = listFileName;

	for( QMap< QString, QString >::iterator iter = settings.keys.begin();
		iter != settings.keys.end(); iter++ )
	{
		args.append(iter.key());
		args.append(*iter);
	}

	process->setWorkingDirectory(path);
	process->start(settings.programmFileName,args);
	
	bool res = false;
	if ( process->waitForStarted() )
	{
		process->waitForFinished();

		if ( process->exitStatus() == QProcess::NormalExit && 
			 !process->exitCode() && 
			(settings.keys.find("--MLF-file") != settings.keys.end()) )
		{
			outData.Marking.clear();
			res = GetMarking(path + "/" + *settings.keys.find("--MLF-file"), 
							outData.Marking, 
							lastErrorDesc, 
							settings.ShowBest,
							settings.StartThreshold,
							settings.EndThreshold);
		}
	}
	else 
		lastErrorDesc = QString::fromLocal8Bit("Неудалось запустить процесс, ошибка: %1").arg(GetProcessError(process->error()));

	std::cout << process->readAllStandardOutput().data() << std::endl;

	delete process;
	QFile::remove(path + "/" + listFileName);
	return res;
}


bool latticePlugin::InitBatchProcessing(void * & internalData, int id, const QString & settingsStr, QString & errorDesc)
{
	if ( !GetSettings(settingsStr, settings, errorDesc) )
		return false;
	return true;
}

bool latticePlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & lastErrorDesc)
{
	QProcess * process = new QProcess();
	QStringList args;
	QString path = QFileInfo(settings.programmFileName).absolutePath();

	//! Создадим свой лист файл.
	QString listFileName(QUuid::createUuid().toString() + ".list");
	QFile listFile(path + "/" + listFileName);
	QMap<QString, QString> keys = settings.keys;

	if ( listFile.open(QIODevice::WriteOnly) )
	{
		listFile.write(fileDesc.first.toLocal8Bit().data());
		listFile.close();
	}
	else 
	{
		lastErrorDesc = QString::fromLocal8Bit("Немогу создать файл со списком латисов: \"%1\"").arg(listFileName);
		return false;
	}

	keys["--list-file"] = listFileName;
	keys["--MLF-file"] = fileDesc.second;

	for( QMap< QString, QString >::iterator iter = keys.begin();
		iter != keys.end(); iter++ )
	{
		if ( !iter.key().isEmpty() )
		{
			args.append(iter.key());
			args.append(*iter);
		}
	}

	process->setWorkingDirectory(path);
	process->start(settings.programmFileName,args);
	if ( !process->waitForStarted() )
	{
		lastErrorDesc = QString::fromLocal8Bit("Неудалось запустить процесс, ошибка: %1").arg(GetProcessError(process->error()));
		QFile::remove(path + "/" + listFileName);
		delete process;
		return false;
	}

	process->waitForFinished();
	bool res = false;

	if ( process->exitStatus() == QProcess::NormalExit)
	{
		if ( res = !process->exitCode() )
		{
			//! Обработаем файл результата.
			if ( settings.UsePostProcessing )
			{
				QList<SUPPhraseData> marking;
				if ( res = GetMarking(fileDesc.second, marking, lastErrorDesc, settings.ShowBest,
								settings.StartThreshold, settings.EndThreshold) )
				{
					QFile::remove(fileDesc.second);
					res = SaveMarkingInFile(fileDesc.second, marking, lastErrorDesc);
				}
			}
		}
		else 
			lastErrorDesc = QString::fromLocal8Bit("Процесс \"%1\" завершился с ошибкой!!").arg(settings.programmFileName);
	}
	else
		lastErrorDesc = QString::fromLocal8Bit("Процесс \"%1\" упал!!").arg(settings.programmFileName);


	delete process;
	QFile::remove(path + "/" + listFileName);
	return res;
}

void latticePlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	return;
}


bool latticePlugin::GetMarking(const QString & fileName, 
							   QList<SUPPhraseData> & Marking, 
							   QString & errorDesc,
							   bool showBest,
							   int startThreshold, 
							   int endThreshold)
{
	QFile file (fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		errorDesc = QString::fromLocal8Bit("Немогу открыть файл разметки: \"%1\"").arg(fileName);
		return false;
	}

	QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > > list;
	const qreal sampleRate = 8000;

	while (!file.atEnd()) 
	{
		QString line = QString::fromLocal8Bit(file.readLine().data());
		QRegExp regExp("(\\d+)\\s+(\\d+)\\s+(\\w+)\\s+(-?\\d+[\\.\\,]?\\d*)");

		if ( regExp.indexIn(line) != -1 )
		{
		 list.append(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> >(
			 QPair<qlonglong, qlonglong>(regExp.cap(1).toLongLong(), regExp.cap(2).toLongLong()),
			 QPair<QString, qreal>(regExp.cap(3), regExp.cap(4).toDouble())) );
		}
	}

	// * 0.0000001) * sampleRate
	if ( !showBest )
	{
		for ( QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > >::iterator iter = list.begin();
			iter != list.end(); iter++ )
		{
			Marking.append(SUPPhraseData(((qreal)iter->first.first* 0.0000001) * sampleRate, 
										((qreal)iter->first.second* 0.0000001) * sampleRate, 
										iter->second.first));
		}
	}
	else 
	{

		QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > best;

		for ( QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > >::iterator iter = list.begin();
			iter != list.end(); iter++ )
		{
            if ( phrase_in_best(&(*iter), best, startThreshold, endThreshold) )
                continue;
            GetBestPhrase(&(*iter), list, startThreshold, endThreshold, best);
		}

		//! Отображаем разметку.
		for ( QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * >::iterator iter = best.begin();
				iter != best.end(); iter++ )
		{
			Marking.append(SUPPhraseData(((qreal)(*iter)->first.first* 0.0000001) * sampleRate, 
										((qreal)(*iter)->first.second* 0.0000001) * sampleRate, 
										(*iter)->second.first));
		}
	}
	
	if ( !Marking.size() )
	{
		errorDesc = QString::fromLocal8Bit("Разметка не создана, файл: \"%1\"").arg(fileName);
		return false;
	}
	return true;
}


void latticePlugin::GetBestPhrase(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * phrase,
				   QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > > & list,
				   qlonglong Start_Threshold, 
				   qlonglong End_Threshold,
				   QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > & best)
{
	for ( QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > >::iterator iter = list.begin();
		iter != list.end(); iter++ )
	{
		if ( (phrase->first.first <= iter->first.first + Start_Threshold) && (phrase->first.first >= iter->first.first - Start_Threshold) &&
			 (phrase->first.second <= iter->first.second + End_Threshold) && (phrase->first.second >= iter->first.second - End_Threshold) &&
			 (phrase->second.second <= iter->second.second) )
		{
			phrase = &(*iter);
		}
	}
	best.append(phrase);
}

bool latticePlugin::phrase_in_best(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * phrase, 
					QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > & list,
					qlonglong Start_Threshold,
					qlonglong End_Threshold )
{
	for ( QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * >::iterator iter = list.begin();
		iter != list.end(); iter++ )
	{
		if ( (phrase->first.first <= (*iter)->first.first + Start_Threshold) && (phrase->first.first >= (*iter)->first.first - Start_Threshold) &&
			 (phrase->first.second <= (*iter)->first.second + End_Threshold) && (phrase->first.second >= (*iter)->first.second - End_Threshold) &&
			 (phrase->second.second <= (*iter)->second.second) )
		{
			return true;
		}
	}
	return false;
}


Q_EXPORT_PLUGIN2(latticePlugin, latticePlugin);