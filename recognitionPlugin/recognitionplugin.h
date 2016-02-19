#ifndef RECOGNITION_PLUGIN_H
#define RECOGNITION_PLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"

using namespace MoRe;

//! Реализация плагина распознования языков.
class RecognitionPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	RecognitionPlugin();

	//! Деструктор.
	~RecognitionPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; };

	//! Получить имя плагина.
	QString GetPluginName() const { return QString::fromLocal8Bit("Распознование языков."); }

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes() const{return SUPluginInterface::BatchProcessing; }

	//! получить диалог настройки.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec(); }

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "RecognitionPlugin.dll";}

protected:
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
		QString PauseModel;
		QString SpeechModel;
		QString NoiceModels;
		QString LanguagesModels;
		QString PhonemModels;
	};

	//! Используется для хранения данных потока ибработки файов.
	struct BatchThreadData
	{
		Settings settings;
	};

	QMenu * menu;
	SettingsDialog * dialog;
	bool isInit;

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
};

#endif // RECOGNITION_PLUGIN_H
