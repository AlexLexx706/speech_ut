#ifndef VADMARKINGPLUGIN_H
#define VADMARKINGPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "../json_spirit/json_spirit.h"
#include <QLibrary>
#include "settingsdialog/settingsdialog.h"
#include <Speaker Identification\SDiSeCEngine.h>
#include <Models Processing\GMM\eGmmTk.h>


//! Реализация базого плагина.
class SUVadPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	SUVadPlugin();

	//! Деструктор.
	~SUVadPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const {return isInit;}

	//! Получить имя плагина.
	QString GetPluginName() const{return "Plugin_VAD";}

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes() const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing; }

	//! получить диалог настройки.
	void OpenSettingsDialog();
	
	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "VadMarkingPlugin.dll";}


protected:
	//! Функция вызывается потокими рассчитывающие разметку.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);
	
	/** 
	 * Вызывается перед началом потоковой обработки, инициализация ресурсов
	 * @param id
	 * @param files_reports [out] - Функция должна заполнить список QPair<файл на обработку, файл отчёта>
	 * @param errorDesc [out] - описание ошибки.
	 * @result - результат.
	 */
	bool InitBatchProcessing(void * & internalData, int id,  const QString & settings, QString & errorDesc);

	/**
	 * Вызывается для каждого файла.
	 * @param id [in] - id передаётся в плагин приложением.
	 * @param fileDesc [in] - содержит QPair<имя файла, имя файла отчёта>, файл отчёта должен быть создан в данной функции.
	 * @param errorDesc [out] - описание ошибки.
	*/
	bool FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc);

	/** 
	 * Вызывается после завершения обработки, освобождение ресурсов.
	 * @param id [in] - ID
	*/
	void ReleaseBatchProcessing(void * & internalData, int id);

	//Переименовать имена событий.
	virtual void RenameActionsNames(QList< QAction *> & actions);

private:
	struct Settings
	{
		QString ModiffedFilesDir;
		bool	CutNoice;
		QString vadParams;

		QString UBMModelPath;
		QString PauseDCModelPath;
		QString SpeechDCModelPath; 
		bool UseDictorCount;
		double tresholdParam_dc;
		int LargeWinSize;
		QPair<bool, QMap<QString, bool> > cut_models;
	};

	struct HMSData
	{
		SDiSeCEngine sDiSeC;
		eGmmTk gmmTk;
		SP_BYTE* UBM;
		SP_BYTE* speechModel;
		SP_BYTE* nonSpeechModel;
		HMSData():UBM(NULL), speechModel(NULL), nonSpeechModel(NULL){}
	};

	struct ThreadData
	{
		VAD_GMM::VadSettings vadModel;
		Settings settings;
		HMSData hsmData;
	};

	QMenu * menu;
	SettingsDialog * dialog;
	bool isInit;

	//! Основная функция.
	bool doVAD(const QByteArray & signal, QList<SUPPhraseData> & marks);

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);

	bool LoadHMSModels(HMSData & data, const Settings & settings, QString & lastErrorDesc);
	bool FreeHMSModels(HMSData & data);
	bool HowManySpeakers(const QByteArray & signal,
								  QList<SUPPhraseData> & marking,
								  SUVadPlugin::HMSData & hmsData,
								  double tresholdParam_dc,
								  int LargeWinSize,
								  QString & lastErrorDesc);
	void RemoveSignalsFromSound(const QPair<bool, QMap<QString, bool> > & cut_models,
						QList<SUPPhraseData> & marking, 
						QByteArray & sound);
	void RemoveNoiseFromSound(QList<SUPPhraseData> & marking, QByteArray & sound);
};

#endif // VADMARKINGPLUGIN_H
