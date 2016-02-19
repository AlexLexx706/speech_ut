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

	//! ������� ��������.
	void StartGetMarking(int id, const SUPInputData & inData);

	//! ���������� ����.
	void StartProcessingFile(int id, const QString & settings);

	//! ���������� ������ �������.
	void StartExitServer();

public slots:
	//! ���������� ���������.
	void StopExternalProcessing();

private:
	InternalThread & thread;
	QByteArray replySize;
	QByteArray reply;

private slots:	
	void OnReadyRead();
};

#endif // PLUGINSOCKET_H
