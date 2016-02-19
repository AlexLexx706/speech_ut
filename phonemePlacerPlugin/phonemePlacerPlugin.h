#ifndef CNDVITERBIPLUGIN_H
#define CNDVITERBIPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <QMutex>

//! Реализация базого плагина.
class PhonemePlacerPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	PhonemePlacerPlugin();

	//! Деструктор.
	~PhonemePlacerPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; }

	//! Получить имя плагина.
	QString GetPluginName() const {return "Plugin_PhonemePlacer";}

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const {return MakeMarking | BatchProcessing;}

	//! получить диалог настройки.
	void OpenSettingsDialog(){if (dialog) dialog->exec();}

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "PhonemePlacerPlugin.dll";}

protected:
	//! Функция вызывается потокими рассчитывающие разметку.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! Функция вызывается перед эмитом сигнала MakeMarkingFinished, рузультат функции передаётся в MakeMarkingFinished.
	bool PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc);

	/**
	 * Начало пакетной обработки.
	 * Вызывает в потоке пакетной обработки в его начале, используется для инициализации общих ресурсов.
	 * @param id [in] - параметр преданный пользователем.
	 * @param settings [in] - настройки.
	 * @param errorDesc [out] - описание ошибки.
	*/
	virtual bool BeginBatchProcessing(int id, const QString & settings, QString & errorDesc);

	/**
	 * Завершение пакетной обработки.
	 * Вызывает в потоке пакетной обработки в его конце, используется для освобождения общих ресурсов.
	 * @param id [in] - параметр преданный пользователем.
	*/
	virtual void EndBatchProcessing(int id);

	/** 
	 * Вызывается для каждого потока, при его старте.
	 * @param id
	 * @param files_reports [out] - Функция должна заполнить список QPair<файл на обработку, файл отчёта>
	 * @param errorDesc [out] - описание ошибки.
	 * @result - результат.
	 */
	virtual bool InitBatchProcessing(void * & internalData, int id, const QString & settings, QString & errorDesc);

	/** 
	 * Вызывается для каждого потока после завершения обработки файлов.
	 * @param id [in] - ID
	*/
	virtual void ReleaseBatchProcessing(void * & internalData, int id);


	/**
	 * Вызывается для каждого файла.
	 * @param id [in] - id передаётся в плагин приложением.
	 * @param fileDesc [in] - содержит QPair<имя файла, имя файла отчёта>, файл отчёта должен быть создан в данной функции.
	 * @param errorDesc [out] - описание ошибки.
	*/
	virtual bool FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc);

	//Переименовать имена событий.
	virtual void RenameActionsNames(QList< QAction *> & actions);

private:
	struct Settings
	{
		QString HmmModel;
		QString TextFileName;
		bool	PhraseIsCorrect;
		QString TranscriptionsDir;
		bool is_output_type_by_states;
	};

	//! Используется для хранения данных потока ибработки файов.
	struct BatchThreadData
	{
		Settings settings;
		int placer_id;
	};

	SettingsDialog * dialog;
	QMenu * menu;
	bool isInit;

	QMap<int, int> model_map;
	QMutex model_map_locker;

	//! Загрузка транскрипции из файла.
	bool LoadTranscription(const QString & fileName, std::string & transcription, QString & lastErrorDesc);

	//! Создать строку описывающую дополнительный результат.
	QString CreateAdditionResult(bool phraseIsCorrect);

	//! Добавить разметку.
	void AddMarking(long framesCount, const std::list<std::pair<std::string, int> > & inResult, long offset, QList<SUPPhraseData> & marking);

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
	
	bool GetModelId(int id, int &model_id, bool remove_id);
};

#endif // CNDVITERBIPLUGIN_H
