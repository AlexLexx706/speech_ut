#ifndef LATTICE_PLUGIN_H
#define LATTICE_PLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"


//! Реализация базого плагина.
class latticePlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	latticePlugin();

	//! Деструктор.
	~latticePlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; }

	//! Получить имя плагина.
	QString GetPluginName() const {return "latticePlugin";}

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const {return BatchProcessing;}

	//! получить диалог настройки.
	void OpenSettingsDialog(){if (dialog) dialog->exec();}

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "latticePlugin.dll";}

	QString GetBatchFileProcessngMask() { return "*.lat";};

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
		QString programmFileName;
		QMap< QString, QString > keys;
		int StartThreshold;
		int EndThreshold;
		bool ShowBest;
		QString latticeFile;
		bool UsePostProcessing;
	};

	SettingsDialog * dialog;
	QMenu * menu;
	QString transcriptionsDir;
	bool isInit;
	Settings settings;


	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);

	//! Получить разметку из файла.
	bool GetMarking(const QString & fileName, 
					QList<SUPPhraseData> & Marking, 
					QString & errorDesc, 
					bool showBest,
					int startThreshold, 
					int endThreshold);


	//! Получить лутьшую
	static void GetBestPhrase(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * phrase,
					   QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > > & list,
					   qlonglong Start_Threshold, 
					   qlonglong End_Threshold,
					   QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > & best);

	static bool phrase_in_best(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * phrase, 
						QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > & list,
						qlonglong Start_Threshold,
						qlonglong End_Threshold );


};

#endif // LATTICE_PLUGIN_H
