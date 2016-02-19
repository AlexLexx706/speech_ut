#ifndef LANGUAGEIDENTIFICATIONPLUGIN_H
#define LANGUAGEIDENTIFICATIONPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <QMutex>


//! Реализация базого плагина.
class LanguageIdentificationPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	LanguageIdentificationPlugin();

	//! Деструктор.
	~LanguageIdentificationPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; }

	//! Получить имя плагина.
	QString GetPluginName() const {return QString::fromUtf8("Фонотактический распознователь языков");}

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const {return MakeMarking | BatchProcessing;}

	//! получить диалог настройки.
	void OpenSettingsDialog(){if (dialog) dialog->exec();}

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "LanguageIdentificationPlugin.dll";}
	virtual QString GetBatchFileResultMask() { return ".txt";}

protected:
	//! Функция вызывается потокими рассчитывающие разметку.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! Функция вызывается перед эмитом сигнала MakeMarkingFinished, рузультат функции передаётся в MakeMarkingFinished.
	bool PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc);


	//! Начало пакетной обработки.
	bool BeginBatchProcessing(int id, const QString & settings, QString & errorDesc);

	//! Завершение пакетной обработки.
	void EndBatchProcessing(int id);

	/** 
	 * Вызывается перед началом потоковой обработки, инициализация ресурсов
	 * @param id
	 * @param files_reports [out] - Функция должна заполнить список QPair<файл на обработку, файл отчёта>
	 * @param errorDesc [out] - описание ошибки.
	 * @result - результат.
	 */
	bool InitBatchProcessing(void * & internalData, int id, const QString & settingsStr, QString & errorDesc);

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
		struct ModelsDesc
		{
			std::string channel;
			std::vector<const char*> models;
			std::list<std::string> __models;
		};

		bool useMoreServer;
		QString moreServerHost;
		int moreServerPort;

		QString outDir;
		QString pauseModelPath;
		QString speechModelPath;
		QString modelsDir;
		QString modelsMask;

		bool saveRecognitionReport;
		bool sortFilesByLangs;
		bool createLabFile;


		QString nGrammModelsPath;
		QString nGrammModelsMask;
		QString nGrammModelsMask_aux;

		QString phonemsModelsPath;
		QString phonemsModelsMask;
		bool noPhonemeChannels;

		bool useVad;
		bool useTxtModel;
		int grammCount;
		double threshold;

		ModelsDesc languages;
		ModelsDesc phonems;
		bool useOpenTask;

		QString losersDirName;
	};

	//! Используется для хранения данных потока ибработки файов.
	struct BatchThreadData
	{
		Settings settings;
		int client;
		int serverID;
		BatchThreadData():client(-1), serverID(0){}
	};

	SettingsDialog * dialog;
	QMenu * menu;
	bool isInit;

	QMutex batchLocker;
	QMap<int, BatchThreadData> batchProcDescs;
	int serverCount;

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
	
	bool getModels(QString path,
					QString mask,
					QByteArray & bufferInModels,
					QString & jSonPar,
					QString & errorDesc);
	
	bool loadVADmodels(const QString & pauseModelPath,
						const QString & speechModelPath,
						const QString & modelsPath,
						const QString & masks,
						int clientID,
						QString & errorDesc);
	
	bool SaveRes(const std::string & res,
				const QPair<QString, QString> & fileDesc,
				const QByteArray & sound,
				const Settings & settings,
				QString & lastErrorDesc);

	bool InitRecognition(bool batchProcessing,
						const QString & settingsStr,
						Settings & outSettings,
						int & outClientId,
						int & serverID,
						QString & errorDesc);

	bool GetWinner(const std::string & recResultsStr,
					std::list<std::string> & winners,
					std::list<std::string> & losers,
					QString & lastErrorDesc,
					std::string & outReport,
					const Settings & settings);
};

#endif // LANGUAGEIDENTIFICATIONPLUGIN_H