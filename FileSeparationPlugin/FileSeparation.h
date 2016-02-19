#ifndef CNDVITERBIPLUGIN_H
#define CNDVITERBIPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"


//! Реализация базого плагина.
class FileSeparationPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	FileSeparationPlugin();

	//! Деструктор.
	~FileSeparationPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; }

	//! Получить имя плагина.
	QString GetPluginName() const {return QString::fromUtf8("Расщепление файлов по разметкам");}

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const {return MakeMarking | BatchProcessing;}

	//! получить диалог настройки.
	void OpenSettingsDialog(){if (dialog) dialog->exec();}

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "FileSeparationPlugin.dll";}

protected:
	//! Функция вызывается потокими рассчитывающие разметку.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! Функция вызывается перед эмитом сигнала MakeMarkingFinished, рузультат функции передаётся в MakeMarkingFinished.
	bool PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc);

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
		QString outDir;
		bool markingAsUtf8;
	};

	//! Используется для хранения данных потока ибработки файов.
	struct BatchThreadData
	{
		Settings settings;
	};

	SettingsDialog * dialog;
	QMenu * menu;
	bool isInit;

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
	bool LoadMarking(const QString & fileName,
					bool markingAsUtf,
					QList<QPair<QString, QPair<long, long> > > & marking,
					QString & lastErrorDesc);
};

#endif // CNDVITERBIPLUGIN_H
