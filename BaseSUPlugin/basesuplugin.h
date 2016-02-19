#ifndef BASESUPLUGIN_H
#define BASESUPLUGIN_H

#include <QObject>
#include "basesuplugin_global.h"
#include "plugincommon.h"
#include <QFile>
#include <QTextStream>

class InternalThread;
class QDialog;
class QMutex;
class QAction;

//! Реализация базого плагина.
/**
	Формат стандартных настроек плагина в формате JSON, передающихся в функции: 
		StartMakeMarking, StartBatchProcessing:
		{
			pluginName:"xxx",
			parameters:{
				reportsDir:"",
				processingDir:"",
				maxBPThreadCount:2
				useExternalProcessing: true
			}
		}
		pluginName - имя плагина.
		parameters - параметры плагина.
		reportsDir - имя папки для сохранения разметки.
		processingDir - имя папки с файлами на обработку.
		maxBPThreadCount - максимальное количество потоков.
		useExternalProcessing - обрабатывать данные во внешнем процесе.
*/

class BASESUPLUGIN_EXPORT BaseSUPlugin:public QObject, public SUPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(SUPluginInterface)
public:
	//! Конструктор.
	BaseSUPlugin();

	//! Деструктор.
	~BaseSUPlugin();

	//! Инициализация плагина.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar) { return false; }

	//! Завершение работы с плагином
	bool Free() { return false; }

	//! Состояние плагина.
	bool IsInit() const { return false; }

	//! Получить имя плагина.
	QString GetPluginName() const {return "Base plugin";}

	//! Получить действия поддерживаемые плагином.
	int GetActionsTypes()const {return MakeMarking;}

	QString GetLastErrorDesc() const { return lastErrorDesc; }

	//! Получить список результатов после пакетной обработки.
	void GetBatchResult(QList<SUPFileResult> & result);

	//! Получить общий результат пакетной обработки.
	bool GetCommonBatchResult();

	//! Получить настройки плагина.
	QString GetPluginSettings() const {return "";}

	//! Получить имя dll плагина.
	virtual QString GetLibName() const = 0;

	virtual QString GetBatchFileProcessngMask() { return "*.wav";};
	virtual QString GetBatchFileResultMask() { return ".lab";}

public slots:
	//! получить диалог настройки.
	void OpenSettingsDialog(){}

	//! Запустить создание разметки, не блокирующая функция, должна пораждать поток создания разметки. 
	bool StartMakeMarking(int ID, const SUPInputData & inData);

	//! Запустить пакетную обработку данных.
	bool StartBatchProcessing(int ID, const QString & settings);
	
	//! Остановить пакетную обработку данных.
	bool StopBatchProcessing(int ID);

signals:
	//! Создание разметки начато.на обработку
	void MakeMarkingStarted(int ID);

	//! Создание разметки завершено.
	void MakeMarkingFinished(int ID, const SUPOutData & data, bool result, const QString & errorDesc);

	//! Запуск пакетной обработки.
	void BatchProcessingStarted(int ID);

	//! Пакетная обработка завершена.
	void BatchProcessingFinished(int ID, bool result, const QString & errorDesc);

	//! Прогресс пакетной обработки.
	void ProgressMessage(int id, qreal progress, const QString & message);

	//! Начала обработки файла.
	void StartFileProcessing(int id, const QString & fileName);

	//! Результат обработки файла.
	void FileProcessingResult(int id, const SUPFileResult & result);

	//! Сигнал получить разметку.
	void GetMarkingTriggered();

	//! Сигнал получить разметку.
	void GetMarkingOnMarkingTriggered();

	//! Сигнал запустить пакетную обработку.
	void StartBatchProcessingTriggered();

	//! Сигнал остановить пакетную обработку.
	void StopBatchProcessingTriggered();

protected:
	friend class InternalThread;
	friend class BatchThread;

	//! Описание оштбки.
	QString lastErrorDesc;

	//! Функция вызывается потокими рассчитывающие разметку.
	virtual bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! Функция вызывается перед эмитом сигнала MakeMarkingFinished, рузультат функции передаётся в MakeMarkingFinished.
	virtual bool PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc);

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
	 * Вызывается для каждого файла.
	 * @param id [in] - id передаётся в плагин приложением.
	 * @param fileDesc [in] - содержит QPair<имя файла, имя файла отчёта>, файл отчёта должен быть создан в данной функции.
	 * @param errorDesc [out] - описание ошибки.
	*/
	virtual bool FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc);

	/** 
	 * Вызывается для каждого потока после завершения обработки файлов.
	 * @param id [in] - ID
	*/
	virtual void ReleaseBatchProcessing(void * & internalData, int id);


	//! Сохранить разметку в файл.
	static bool SaveMarkingInFile(const QString & fileName, const QList<SUPPhraseData> & marking, QString & errorDesc);

	//! Загрузить данные из wav файла определенного формата.
	static bool LoadFile(const QString & fileName, int bitPerSample, int samplerate, QByteArray & signal, QString & errorDesc);

	//! Записать данные в файл.
	static bool SaveFile(const QString & fileName, int cannels, int bitPerSample, int samplerate, QByteArray & signal, QString & errorDesc);

	QMenu * InitActions(QMenu * inMenu, QToolBar * inToolBar, unsigned int actionsMask = 0x1f);

	//Переименовать имена событий.
	virtual void RenameActionsNames(QList< QAction *> & actions);

private:
	QList<InternalThread *> threads;

	InternalThread * batchProcessingThread;

	QList<SUPFileResult> batchResult;

	QMutex * batchResultMutex;

	QFile completedFiles;
	QTextStream completedFilesStream;

	QFile errorCompletedFiles;
	QTextStream errorCompletedFilesStream;

	InternalThread * GetFreeThread();

	void EmitStartSignal(int ID, SUPluginInterface::ActionsType type);

	void FileProcessingInternal(void * internalData, int id, const QPair<QString, QString> & file_report);

	void EmitFileProcessingResult(int id, const SUPFileResult & res);

	void EmitProgressMessage(int id, qreal progress, const QString & message);

	void EmitStartFileProcessing(int id, const QString & fileName);

	//! Количество обработанных файлов.
	int GetCompletedFilesCount() const;

	//! Общее количество обрабатываемых файлов.
	int GetFilesCount() const;

	//! Получить общие настройки для потоковой обработки.
	bool GetCommonBatchSettings(const QString & settings,
										QList< QPair<QString, QString> > & files,
										int & maxBPThreadCount,
										QString & errorDesc);
	
	//! Использовать внешнюю обработку, в отдельном процессе.
	bool IsUseExternalProcessing(const QString & settings, QString & errorDesc);

	//! Подготовка настроек для удалённого исполнения.
	bool PrepareForExernalProcessing(const QString & settings, QString & outSettings, QString & errorDesc);

private slots:
	void OnThreadFinished();
};

//! Опрераторы для сериализации.
BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPPhraseData & data);

//! Опрераторы для сериализации.
BASESUPLUGIN_EXPORT QDataStream & operator >>(QDataStream & out, SUPPhraseData & data);

BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPInputData & data);

BASESUPLUGIN_EXPORT QDataStream & operator >> (QDataStream & out, SUPInputData & data);

BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPOutData & data);

BASESUPLUGIN_EXPORT QDataStream & operator >> (QDataStream & out, SUPOutData & data);

BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPFileResult & data);

BASESUPLUGIN_EXPORT QDataStream & operator >> (QDataStream & out, SUPFileResult & data);


#endif // BASESUPLUGIN_H
