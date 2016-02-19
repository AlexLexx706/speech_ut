#include "BasePluginServer.h"
#include <QDataStream>
#include <QLocalSocket>
#include <iostream>
#include <QDataStream>
#include <QCoreApplication>
#include "../../BaseSUPlugin/basesuplugin.h"
#include "basepluginservercommand.h"
#include <QTcpSocket>

BasePluginServer::BasePluginServer(SUPluginInterface & _plugin, QObject *parent):
	QTcpServer(parent),
	clientNumber(0),
	commandNumber(0),
	plugin(_plugin)
{
	connect(this, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));

	connect( dynamic_cast<QObject*>(&plugin), 
			SIGNAL(MakeMarkingFinished(int, const SUPOutData &, bool, const QString &)),
			this, 
			SLOT(OnMakeMarkingFinished(int, const SUPOutData &, bool, const QString &)) );

	connect( dynamic_cast<QObject*>(&plugin), 
			SIGNAL(BatchProcessingFinished(int, bool, const QString &)),
			this, 
			SLOT(OnBatchProcessingFinished(int, bool, const QString &)) );

	connect( dynamic_cast<QObject*>(&plugin), 
			SIGNAL(FileProcessingResult(int, const SUPFileResult &)),
			this, 
			SLOT(OnFileProcessingResult(int, const SUPFileResult &)) );

	connect( dynamic_cast<QObject*>(&plugin), 
			SIGNAL(StartFileProcessing(int, const QString &)),
			this, 
			SLOT(OnStartFileProcessing(int, const QString &)) );

	plugin.Init(NULL, NULL, NULL);
}

BasePluginServer::~BasePluginServer()
{
	plugin.Free();
	close();
}

bool BasePluginServer::IsInit() const
{
	return plugin.IsInit();
}

void BasePluginServer::OnNewConnection()
{
	InternalData data;
	QTcpSocket * socket = nextPendingConnection();
	clients[socket].Number = clientNumber;

	connect(socket, SIGNAL(disconnected()), this, SLOT(OnDeleteSocket()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
	std::cout << "Client " << clientNumber << " connected." << std::endl;
	clientNumber++;
}


void BasePluginServer::OnDeleteSocket()
{
	if ( QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender()) )
	{
		//Удалим все
		QList<int> ids;
		for ( QMap<int, QTcpSocket *>::iterator iter = commandMap.begin(); 
			iter != commandMap.end(); iter++ )
		{
			if ( *iter == socket )
				ids.append(iter.key());
		}
		for(QList<int>::iterator iter = ids.begin(); iter != ids.end(); iter++ )
			commandMap.remove(*iter);


		std::cout << "Client " << clients[socket].Number << " disconnected." <<  std::endl;
		clients.remove(socket);
		socket->deleteLater();
	}
}

bool BasePluginServer::ReadData(QTcpSocket & socket, InternalData & internalData)
{
	if ( internalData.bufferSize.size() < sizeof(int) )
	{
		int size = sizeof(int) - internalData.bufferSize.size();
		internalData.bufferSize += socket.read(socket.bytesAvailable() > size ? size : socket.bytesAvailable());
	}

	//! Читаем данные.
	if ( internalData.bufferSize.size() == sizeof(int) )
	{
		int size = *((int *)internalData.bufferSize.data());
		internalData.buffer += socket.readAll();

		if  ( internalData.buffer.size() == size )
			return true;
	}
	return false;
}


void BasePluginServer::OnReadyRead()
{
	if ( QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender()) )
	{
		InternalData & internalData = clients[socket];
		if ( ReadData(*socket, internalData) )
		{
			QByteArray outBuffer;
			QDataStream stream(internalData.buffer);
			int command;
			stream >> command;

			//! Обработка комманды.
			switch( command )
			{
				case CmdGetMarking:
					GetMarking(command, socket, stream);
					break;
				case CmdBatchProcessing:
					BatchProcessing(command, socket, stream);
					break;
				case CmdExitServer:
					ExitServer(command, socket, stream);
					break;
				case CmdStopProcessing:
					StopProcessing(command, socket, stream);
					break;
				default:
					UnknownCommand(command, socket, stream);
					break;
			}

			//! Освободим буффера комманды.
			internalData.bufferSize.resize(0);
			internalData.buffer.resize(0);	
		}
	}
}

void BasePluginServer::SendSampleResult(QTcpSocket * socket, int cmd, bool res, const QString & desc)
{
	QByteArray outBuffer;
	QDataStream outStream(&outBuffer, QIODevice::WriteOnly);

	outStream << cmd;
	outStream << res;
	outStream << desc;
	int size = outBuffer.size();
	socket->write((const char *)&size, sizeof(size));
	socket->write(outBuffer);
	socket->flush();
	socket->waitForBytesWritten(-1);

}

void BasePluginServer::GetMarking(int cmd, QTcpSocket * socket, QDataStream & stream)
{
	std::cout << "GetMarking Started" << std::endl;
	SUPInputData inData;
	stream >> inData;

	if ( plugin.StartMakeMarking(commandNumber, inData) )
	{
		commandMap[commandNumber] = socket;
		commandNumber++;
	}
	else
	{
		QByteArray outBuffer;
		QDataStream outStream(&outBuffer, QIODevice::WriteOnly);
		outStream << CmdGetMarking;
		outStream << false;
		outStream << QString::fromLocal8Bit("Ошибка запуска комманды");
		outStream << SUPOutData();

		int size = outBuffer.size();
		socket->write((const char *)&size, sizeof(size));
		socket->write(outBuffer);
		socket->flush();
		socket->waitForBytesWritten(-1);

		std::cout << "GetMarking Completed with error!" << std::endl;
	}
}

void BasePluginServer::BatchProcessing(int cmd, QTcpSocket * socket, QDataStream & stream)
{
	std::cout << "BatchProcessing Started" << std::endl;

	QString settings;	
	stream >> settings;

	if ( plugin.StartBatchProcessing(commandNumber, settings) )
	{
		commandMap[commandNumber] = socket;
		commandNumber++;
	}
	else
	{
		std::cout << "BatchProcessing commpleted with error" << std::endl;
		SendSampleResult(socket, cmd, false, QString::fromLocal8Bit("Неудалось выполнить комманду."));
	}
}


void BasePluginServer::OnMakeMarkingFinished(int id, const SUPOutData & data, bool result, const QString & errorDesc)
{
	if ( commandMap.find(id) != commandMap.end() )
	{
		QTcpSocket * socket = commandMap.take(id);

		QByteArray outBuffer;
		QDataStream outStream(&outBuffer, QIODevice::WriteOnly);

		outStream << CmdGetMarking;
		outStream << result;
		outStream << errorDesc;
		outStream << data;

		int size = outBuffer.size();
		socket->write((const char *)&size, sizeof(size));
		socket->write(outBuffer);
		socket->flush();
		socket->waitForBytesWritten(-1);
	}
	std::cout << "GetMarking Completed" << std::endl;
}

void BasePluginServer::OnBatchProcessingFinished(int id, bool result, const QString & errorDesc)
{
	if ( commandMap.find(id) != commandMap.end() )
	{
		QTcpSocket * socket = commandMap.take(id);

		QByteArray outBuffer;
		QDataStream outStream(&outBuffer, QIODevice::WriteOnly);

		outStream << CmdGetMarking;
		outStream << result;
		outStream << errorDesc;

		int size = outBuffer.size();
		socket->write((const char *)&size, sizeof(size));
		socket->write(outBuffer);
		socket->flush();
		socket->waitForBytesWritten(-1);
	}
	std::cout << "BatchProcessingFinished" << std::endl;
}

void BasePluginServer::OnFileProcessingResult(int id, const SUPFileResult & fileResult)
{
	if ( commandMap.find(id) != commandMap.end() )
	{
		QTcpSocket * socket = commandMap[id];

		QByteArray outBuffer;
		QDataStream outStream(&outBuffer, QIODevice::WriteOnly);

		outStream << CmdEndFileProcessing;
		outStream << fileResult;

		int size = outBuffer.size();
		socket->write((const char *)&size, sizeof(size));
		socket->write(outBuffer);
		socket->flush();
		socket->waitForBytesWritten(-1);
	}
	std::cout << "File:" << fileResult.FileName.toStdString() << " completed" << std::endl;
}

void BasePluginServer::OnStartFileProcessing(int id, const QString & fileName)
{
	if ( commandMap.find(id) != commandMap.end() )
	{
		QTcpSocket * socket = commandMap[id];

		QByteArray outBuffer;
		QDataStream outStream(&outBuffer, QIODevice::WriteOnly);

		outStream << CmdStartFileProcessing;
		outStream << fileName;

		int size = outBuffer.size();
		socket->write((const char *)&size, sizeof(size));
		socket->write(outBuffer);
		socket->flush();
		socket->waitForBytesWritten(-1);
	}
	std::cout << "File:" << fileName.toStdString() << " started" << std::endl;
}


void BasePluginServer::ExitServer(int cmd, QTcpSocket * socket, QDataStream & stream)
{	
	std::cout << "ExitServer Start" << std::endl;
	SendSampleResult(socket, cmd, true, QString::fromLocal8Bit("Сервер выключается!!"));
	QCoreApplication::quit();
}

void BasePluginServer::StopProcessing(int cmd, QTcpSocket * socket, QDataStream & stream)
{
	plugin.StopBatchProcessing(0);
	SendSampleResult(socket, cmd, true, QString::fromLocal8Bit("Поток отключается !!"));
}


void BasePluginServer::UnknownCommand(int cmd, QTcpSocket * socket, QDataStream & stream)
{
	SendSampleResult(socket, cmd, false, QString::fromLocal8Bit("Комманда не реализованна!!"));
}