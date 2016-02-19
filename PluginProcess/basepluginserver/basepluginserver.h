#ifndef BASEPLUGINSERVER_H
#define BASEPLUGINSERVER_H

#include <QTcpServer>

#ifndef BASEPLUGINSERVER_GLOBAL_H
#define BASEPLUGINSERVER_GLOBAL_H

#include <QGlobal.h>

#ifdef BASEPLUGINSERVER_LIB
# define BASEPLUGINSERVER_EXPORT Q_DECL_EXPORT
#else
# define BASEPLUGINSERVER_EXPORT Q_DECL_IMPORT
#endif

#endif // BASEPLUGINSERVER_GLOBAL_H

class SUPluginInterface;
struct SUPOutData;
struct SUPFileResult;

class BASEPLUGINSERVER_EXPORT BasePluginServer : public QTcpServer
{
	Q_OBJECT

public:
	enum ErrorCodec
	{
		NoError = 0,
		Error = 1
	};

	BasePluginServer(SUPluginInterface & plugin, QObject *parent = NULL);
	~BasePluginServer();

	bool IsInit() const;

protected:
	void GetMarking(int cmd, QTcpSocket * socket, QDataStream & stream);
	void BatchProcessing(int cmd, QTcpSocket * socket, QDataStream & stream);
	void ExitServer(int cmd, QTcpSocket * socket, QDataStream & stream);
	void StopProcessing(int cmd, QTcpSocket * socket, QDataStream & stream);
	void UnknownCommand(int cmd, QTcpSocket * socket, QDataStream & stream);

private slots:
	void OnNewConnection();
	void OnDeleteSocket();
	void OnReadyRead();
	
	//! Создание разметки завершено.
	void OnMakeMarkingFinished(int ID, const SUPOutData & data, bool result, const QString & errorDesc);

	//! Пакетная обработка завершена.
	void OnBatchProcessingFinished(int ID, bool result, const QString & errorDesc);

	//! Результат обработки файла.
	void OnFileProcessingResult(int ID, const SUPFileResult & result);

	void OnStartFileProcessing(int id, const QString & fileName);


private:
	struct InternalData
	{
		int Number;
		QByteArray buffer;
		QByteArray bufferSize;
		InternalData():Number(0){}
	};

	QMap< QTcpSocket *, InternalData> clients;
	QMap<int, QTcpSocket *> commandMap;
	int clientNumber;
	int commandNumber;
	SUPluginInterface & plugin;
	bool ReadData(QTcpSocket & socket, InternalData & internalData);
	void SendSampleResult(QTcpSocket * socket, int cmd, bool res, const QString & desc);
};

#endif // BASEPLUGINSERVER_H
