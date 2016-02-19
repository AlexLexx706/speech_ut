#ifndef PLUGINSOCKET_H
#define PLUGINSOCKET_H

#include <QTcpSocket>

struct SUPInputData;
struct SUPOutData;
class InternalThread;

class PluginSocket : public QTcpSocket
{
	Q_OBJECT

public:

	PluginSocket(InternalThread & thread);
	
	~PluginSocket();

	//! Плучить разметку.
	void StartGetMarking(int id, const SUPInputData & inData);

	//! Обработать файл.
	void StartProcessingFile(int id, const QString & settings);

	//! Остановить работу сервера.
	void StartExitServer();

public slots:
	//! Остановить обработку.
	void StopExternalProcessing();

private:
	InternalThread & thread;
	QByteArray replySize;
	QByteArray reply;

private slots:	
	void OnReadyRead();
};

#endif // PLUGINSOCKET_H
