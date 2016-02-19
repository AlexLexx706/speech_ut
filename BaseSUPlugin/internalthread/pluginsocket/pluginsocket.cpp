#include "pluginsocket.h"
#include "../../basesuplugin.h"
#include <QDataStream>
#include "../../PluginProcess/basepluginserver/basepluginservercommand.h"
#include "../internalthread.h"
#include <QDataStream>

PluginSocket::PluginSocket(InternalThread & _thread)
	: thread(_thread)
{
	setReadBufferSize(0);
	connect(this, SIGNAL(readyRead()), this, SLOT(OnReadyRead()), Qt::DirectConnection);
}

PluginSocket::~PluginSocket()
{

}


void PluginSocket::OnReadyRead()
{
	if ( replySize.size() < sizeof(int) )
	{
		int size = sizeof(int) - replySize.size();
		replySize += read(bytesAvailable() > size ? size : bytesAvailable());
	}

	//! Читаем данные.
	if ( replySize.size() == sizeof(int) )
	{
		int size = *((int *)replySize.data());
		reply += readAll();

		if  ( reply.size() == size )
		{
			QDataStream stream(reply);

			//парсим результат.
			int cmd;
			bool res;
			QString resultDesc;
			stream >> cmd;

			switch ( cmd )
			{
				case CmdExitServer:
				{
					stream >> res;
					stream >> resultDesc;
					thread.ReceiveExitServer(res, resultDesc);
					break;
				}
				case CmdGetMarking:
				{
					SUPOutData data;
					stream >> res;
					stream >> resultDesc;
					stream >> data;
					thread.ReceiveGetMarkingReply(res, resultDesc, data);
					break;
				}
				case CmdBatchProcessing:
				{
					stream >> res;
					stream >> resultDesc;
					thread.ReceiveBatchProcessingReply(res, resultDesc);
					break;
				}
				case CmdEndFileProcessing:
				{
					SUPFileResult res;
					stream >> res;
					thread.ReceiveEndFileProcessingReply(res);
					break;
				}
				case CmdStartFileProcessing:
				{
					QString fileName;
					stream >> fileName;
					thread.ReceiveStartFileProcessingReply(fileName);
				}
			}
			
			replySize.resize(0);
			reply.resize(0);
		}
	}
}


void PluginSocket::StartGetMarking(int id, const SUPInputData & inData)
{
	//1. Создадим буффер комманды.
	QByteArray buffer;
	QDataStream inStream(&buffer, QIODevice::WriteOnly);
	inStream << CmdGetMarking;
	inStream << inData;

	//2. Передадим комманду.
	int size = buffer.size();
	writeData((const char *)&size, sizeof(size));
	writeData(buffer.data(), buffer.size());
	flush();
	waitForBytesWritten(-1);

}

void PluginSocket::StartProcessingFile(int id, const QString & params)
{
	//1. Создадим буффер комманды.
	QByteArray buffer;
	QDataStream inStream(&buffer, QIODevice::WriteOnly);
	inStream << CmdBatchProcessing;
	inStream << params;

	//2. Передадим комманду.
	int size = buffer.size();
	writeData((const char *)&size, sizeof(size));
	writeData(buffer.data(), buffer.size());
	flush();
	waitForBytesWritten(-1);

}


void PluginSocket::StartExitServer()
{
	//1. Создадим буффер комманды.
	QByteArray buffer;
	QDataStream inStream(&buffer, QIODevice::WriteOnly);
	inStream << CmdExitServer;

	//2. Передадим комманду.
	int size = buffer.size();
	writeData((const char *)&size, sizeof(size));
	writeData(buffer.data(), buffer.size());
	flush();
	waitForBytesWritten(-1);
}

void PluginSocket::StopExternalProcessing()
{
	QByteArray buffer;
	QDataStream inStream(&buffer, QIODevice::WriteOnly);
	inStream << CmdStopProcessing;

	//2. Передадим комманду.
	int size = buffer.size();
	writeData((const char *)&size, sizeof(size));
	writeData(buffer.data(), buffer.size());
	flush();
	waitForBytesWritten(-1);
}