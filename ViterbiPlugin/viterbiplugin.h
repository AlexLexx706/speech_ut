#ifndef VITERBIPLUGIN_H
#define VITERBIPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <Viterbi/Engine.h>
//#include <Viterbi/definesGlobal.h>
#include <Preprocessing/ppcEngine.h>
#include "../json_spirit/json_spirit.h"



//! Реализация базого плагина.
class ViterbiPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	ViterbiPlugin();

	//! Деструктор.
	~ViterbiPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; };

	//! Получить имя плагина.
	QString GetPluginName() const { return "Plugin_PhonemeRecognition"; }

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes() const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing; }

	//! получить диалог настройки.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec(); }

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "ViterbiPlugin.dll";}

protected:
	//! Функция вызывается потокими рассчитывающие разметку.
	virtual bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	/** 
	 * Вызывается перед началом потоковой обработки, инициализация ресурсов
	 * @param id
	 * @param files_reports [out] - Функция должна заполнить список QPair<файл на обработку, файл отчёта>
	 * @param errorDesc [out] - описание ошибки.
	 * @result - результат.
	 */
	bool InitBatchProcessing(void * & internalData, int id,  const QString & settingsStr, QString & errorDesc);

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
		QString				ModelFileName;
		PRUNING_T 	PruningType;
		SP_REAL				PruningPar;
		LOOPACTIONS_T LoopActionsType;
		bool				UseLangModel;
		int					NGramSize;

		NBEST_T		NBestType;
		int					NBestCount;
		int					NBestCapacity;
		bool				PrintLatticeOutput;
		QString				LatticeOutputFile;
		bool				UseLatticeOutputDir;
		QString				LatticeOutputDir;
		json_spirit::mObject plp;
	};

	//! Используется для хранения данных потока ибработки файов.
	struct BatchThreadData
	{
		MoreEngine batchRecEngine;
		Settings settings;
	};

	QMenu * menu;
	SettingsDialog * dialog;
	bool isInit;
	ppcEngine ppc;

	void AddMarking(int sampleRate, 
					long offset, 
					const std::vector<std::list<std::pair<int, std::string>>> & inResult, 
					QList<SUPPhraseData> & marking);
	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);

	//! Рассчёт фонем.
	bool ViterbiPlugin::ComputePhonemes(MoreEngine & recEngine, 
										 const std::vector<short>  & signal, 
										 const Settings & settings,
										 const char * fileName,
										 std::vector<std::list<std::pair<int, std::string>>> & outputPath,
										 QString & lastErrorDesc);

};

#endif // VITERBIPLUGIN_H
