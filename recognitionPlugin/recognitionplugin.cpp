#include "recognitionplugin.h"
#include <list>
#include <iostream>
#include <string>
#include <QMenu>
#include <QToolBar>
#include <QtPlugin>
#include "../json_spirit/json_spirit.h"
#include <QDir>

using namespace std;


RecognitionPlugin::RecognitionPlugin():BaseSUPlugin(),menu(NULL), dialog(NULL), isInit(false)
{
	lastErrorDesc = QString::fromLocal8Bit("Инициализация не производилась.");
}

RecognitionPlugin::~RecognitionPlugin()
{
	Free();
}

void RecognitionPlugin::RenameActionsNames(QList< QAction *> & actions)
{
	actions[0]->setObjectName("action_setup_Recognition_Plugin");
	actions[1]->setObjectName("action_get_Recognition_marking");
	actions[2]->setObjectName("action_get_Recognition_marking_on_marking");
	actions[3]->setObjectName("action_start_Recognition_batch_processing");
	actions[4]->setObjectName("action_stop_Recognition_batch_processing");
}

bool RecognitionPlugin::Init(QSettings * inSetting, QMenu * inMenu, QToolBar * inToolBar)
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

bool RecognitionPlugin::Free()
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



QString RecognitionPlugin::GetPluginSettings() const
{
	//Передадим дополнительное описание результата.
	json_spirit::Object parameters;

	parameters.push_back(json_spirit::Pair("processingDir", dialog->GetProcessingDir()));
	parameters.push_back(json_spirit::Pair("reportsDir", dialog->GetReportsDir()));
	parameters.push_back(json_spirit::Pair("maxBPThreadCount", dialog->GetMaxBPThreadCount()));
	parameters.push_back(json_spirit::Pair("useExternalProcessing", dialog->UseExternalProcessing()));

	parameters.push_back(json_spirit::Pair("pauseModel", dialog->GetPauseModel()));
	parameters.push_back(json_spirit::Pair("speechModel", dialog->GetSpeechModel()));
	parameters.push_back(json_spirit::Pair("noiceModels", dialog->GetNoiceModels()));
	parameters.push_back(json_spirit::Pair("languagesModels", dialog->GetLanguagesModels()));
	parameters.push_back(json_spirit::Pair("phonemModels", dialog->GetPhonemModels()));

	json_spirit::Object topObj;
	topObj.push_back(json_spirit::Pair("pluginName", GetPluginName().toStdString()));
	topObj.push_back(json_spirit::Pair("parameters", parameters));

	return QString::fromStdString(json_spirit::write(topObj));
}

bool RecognitionPlugin::GetSettings(const QString & str, Settings & settings, QString & errorDesc)
{
	json_spirit::mValue v;
	try 
	{
		if ( json_spirit::read(str.toStdString(), v) && v.get_obj()["pluginName"] == GetPluginName().toStdString() )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();

			settings.PauseModel = QString::fromUtf8(params["pauseModel"].get_str().data());
			settings.SpeechModel = QString::fromUtf8(params["speechModel"].get_str().data());
			settings.NoiceModels = QString::fromUtf8(params["noiceModels"].get_str().data());
			settings.LanguagesModels = QString::fromUtf8(params["languagesModels"].get_str().data());
			settings.PhonemModels = QString::fromUtf8(params["phonemModels"].get_str().data());
			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек: %1").arg(e.what());
	}
	return false;
}


bool RecognitionPlugin::InitBatchProcessing(void * & internalData, int id,  const QString & settingsStr, QString & errorDesc)
{
	internalData = new BatchThreadData;
	BatchThreadData & threadData = *((BatchThreadData *)internalData);

	return false;
}

bool RecognitionPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & file_report, QString & errorDesc)
{
	
	return false;
}

void RecognitionPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
	if ( internalData )
	{
		delete ((BatchThreadData *)internalData);
		internalData = NULL;
	}
}


Q_EXPORT_PLUGIN2(RecognitionPlugin, RecognitionPlugin);
