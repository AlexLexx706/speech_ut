#ifndef INTERNAL_THREAD_H_ALEX
#define INTERNAL_THREAD_H_ALEX

#include <QThread>
#include "../plugincommon.h"
#include "../json_spirit/json_spirit.h"
#include "process/process.h"

class BaseSUPlugin;
class PluginSocket;

//Поток используется для отвязки от потока GUI.
class InternalThread:public QThread
{
	Q_OBJECT
public:
	InternalThread(BaseSUPlugin & _plugin);

	~InternalThread(){}

	bool IsCompleted()const { return !isRunning() && completed;}

	bool SetCompleted();

	bool Start(int _id, const SUPInputData & data, SUPluginInterface::ActionsType _type);

	void Stop();
	
	const SUPOutData & GetOutData() const {return outData;}

	const SUPInputData & GetInData() const {return inData;}

	bool GetResult() const {return res;}

	QString GetLastErrorDesc() const {return lastErrorDesc;}

	SUPluginInterface::ActionsType GetType() const {return type;}

	int GetID() const {return id;}

	int GetCompletedFilesCount() const{ return completedFilesCount; }

	int GetFilesCount() const { return filesCount; }

	void EnableCreateNewProcess(bool e) { enableCreateNewProcess = e;}

	void SetServerParam(const QString & _host, int _port, const QString & _processName = "PluginProcess.exe") 
	{ 
		host = _host;
		port = _port;
		processName = _processName;
	}

	bool IsExternalProcessing() const { return usedExternalProcessing;}


	int MaxRestartCout(){ return maxRestartCount; }
	
	void SetMaxRestartCount(int m){ maxRestartCount = m;}

protected:
	friend class BaseSUPlugin;
	friend class PluginSocket;

	void run();

	//! Функция пакетной обработки.
	bool BatchProcessing();

	//! Обработка разметки во внешнeм процессе.
	void ExternalProcessing(bool res);

	//! Результат комманды GetMarking
	void ReceiveGetMarkingReply(bool result, const QString & resultDesc, SUPOutData & data);

	//! Результат комманды BatchProcessing
	void ReceiveBatchProcessingReply(bool result, const QString & resultDesc);

	//! Результат обработки файла в BatchProcessing. 
	void ReceiveEndFileProcessingReply(const SUPFileResult & fileResult);

	//! Результат комманды остановка сервера.
	void ReceiveExitServer(bool result, const QString & resultDesc);

	//! Результат обработки файла в BatchProcessing. 
	void ReceiveStartFileProcessingReply(const QString & fileName);

	//! Разрыв соединения с сокетом.
	void SocketDisconnected();

private:
	BaseSUPlugin & plugin;
	bool completed;
	bool res;
	QString lastErrorDesc;
	SUPInputData inData;
	SUPOutData outData;
	SUPluginInterface::ActionsType type;
	int id;
	bool isExit;
	int completedFilesCount;
	int filesCount;
	bool enableCreateNewProcess;
	QString host;
	int port;
	QString processName;
	bool usedExternalProcessing;
	Process * process;
	PluginSocket * socket;
	QStringList completedFiles;
	int maxRestartCount;
	int curRestartCount;

	bool GetFreePort(int & port, QString & lastErrorDesc);
	Process * StartNewProcess(const QString & host, int & port, QString & lastErrorDesc);
	PluginSocket * StartNewSocket(const QString & host, int port, QString & lastErrorDesc);
	bool ConnectToServer(PluginSocket & socket, const QString & host, int port, int timeout);
	bool CreateExternalBatchSettings(const QString & inSettings, QString & outSettings, QString & lastErrorDesc);
	void AddFilesDesc(json_spirit::mArray & _array, const QString & fileName, const QString & suffix);

signals:
	void StopExternalProcessing();

private slots:
	void OnDisconnected();
};

#endif // INTERNAL_THREAD_H_ALEX