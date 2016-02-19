#ifndef KWFINDERPLUGIN_H
#define KWFINDERPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <Viterbi/Engine.h>



//! Реализация базого плагина.
class KWFinderPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	KWFinderPlugin();

	~KWFinderPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; }

	//! Получить имя плагина.
	QString GetPluginName() const {return "Plugin_WordRecognition";};

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing;};

	//! получить диалог настройки.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec();}
	
	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "kwFinderPlugin.dll";}

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
		QString PathToHmm;
		QString PathToDict;
		bool dictInUtf8;
		QString PathToTiedls;

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
	};
	
	//! Используется для хранения данных потока обработки файлов.
	struct BatchThreadData
	{
		MoreEngine * batchRecEngine;
		Settings settings;
	};


	SettingsDialog * dialog;
	QMenu * menu;
	bool isInit;

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);	
	
	
	static bool AddMarking(int sampleRate,
							int offset, 
							bool useUtf8,
							const std::vector<std::list<std::pair<int, std::string> > > & result, 
							QList<SUPPhraseData> & marking, 
							QString & lastErrorDesc);

	static bool FindWords(MoreEngine & engine, 
						  const Settings & settings,
						  const char * latticeFileName,
						  const short* signal, 
						  int signalSize,
						  std::vector<std::list<std::pair<int, std::string> > > & outputPath,
						  QString & errorDesc);
	static bool InitEngine(MoreEngine & engine, const Settings & settings, 
							QString & lastErrorDesc);
};


#endif // KWFINDERPLUGIN_H
