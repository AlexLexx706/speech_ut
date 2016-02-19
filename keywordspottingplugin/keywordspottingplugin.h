#ifndef KeywordSpottingPlugin_H
#define KeywordSpottingPlugin_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <Viterbi/Engine.h>




//! Реализация базого плагина.
class KeywordSpottingPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! Конструктор.
	KeywordSpottingPlugin();

	~KeywordSpottingPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! Завершение работы с плагином
	bool Free();

	//! Состояние плагина.
	bool IsInit() const { return isInit; }

	//! Получить имя плагина.
	QString GetPluginName() const {return "Plugin_KeywordSpotting";};

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing;};

	//! получить диалог настройки.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec();}

	//! Получить настройки плагина.
	QString GetPluginSettings() const;

	//! Получить имя dll плагина.
	QString GetLibName() const {return "keywordSpottingPlugin.dll";}

protected:
	//! Функция вызывается потокими рассчитывающие разметку.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! Начало пакетной обработки.
	virtual bool BeginBatchProcessing(int id, const QString & settings, QString & errorDesc);

	//! Завершение пакетной обработки.
	virtual void EndBatchProcessing(int id);


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
	//! Настройки.
	struct Settings
	{
		QString PathToHmm;
		QString PathToDict;
		bool dictInUtf8;
		QString PathToTiedls;
		QString PathToKeywordsDict;
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
	QString commonDictFile;
	QStringList keywords;



	//! Перевести массив в base64
	std::string base64_encode(const unsigned char * buffer, unsigned int len)
	{
		QByteArray temp((const char *)buffer, len);
		return QString(temp.toBase64()).toStdString();	
	}
	static bool AddMarking(int sampleRate, 
								int offset,
								bool dictInUtf8,
								 const std::vector<std::list<std::pair<int, std::string> > > & _result, 
								 const QStringList & keywords,
								 QList<SUPPhraseData> & marking, 
								 QString & lastErrorDesc);

	static bool CreateCommonDict(const QString & dict,
								const QString & keywordsDict, 
								bool dictInUtf8,
								QString & outCommonDict,
								QStringList & outKeywords,
								QString & outErrorDesc);

	static bool IsWordsSimilarity(const QString & w1, const QStringList & w2);

	static bool InitEngine(MoreEngine & engine, const Settings & settings, QString & commonDictFile, QString & lastErrorDesc);

	static bool FindWords(MoreEngine & engine, 
						  const short* signal, 
						  int signalSize, 
						  std::vector<std::list<std::pair<int, std::string> > > & outputPath,
						  QString & errorDesc);

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
};


#endif // KeywordSpottingPlugin_H
