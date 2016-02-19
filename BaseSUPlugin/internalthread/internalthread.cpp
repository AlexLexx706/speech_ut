#include "internalthread.h"
#include "../basesuplugin.h"
#include "batchthread/batchthread.h"
#include <QTcpServer>
#include "pluginsocket/pluginsocket.h"
#include <QTime>
#include <iostream>
#include <stdlib.h>
#include <QDir>


InternalThread::InternalThread(BaseSUPlugin & _plugin): 
	plugin(_plugin), 
	completed(true),
	res(false),
	type(SUPluginInterface::MakeMarking),
	id(0),
	isExit(false),
	completedFilesCount(0),
	filesCount(0),
	enableCreateNewProcess(true),
	host("127.0.0.1"),
	port(60000),
	processName("PluginProcess.exe"),
	usedExternalProcessing(false),
	process(NULL),
	socket(NULL),
	maxRestartCount(10),
	curRestartCount(0)
{}

bool InternalThread::SetCompleted()
{ 
	if (!isRunning())
	{
		completed = true;
		return true;
	}
	return false;
}

bool InternalThread::Start(int _id, const SUPInputData & data, SUPluginInterface::ActionsType _type)
{
	if ( IsCompleted() )
	{
		isExit  = false;
		inData = data;
		res = false;
		type = _type;
		id = _id;
		completedFilesCount = 0;
		filesCount = 0;
		completedFiles.clear();
		usedExternalProcessing = false;
		curRestartCount = 0;

		start();
		return true;
	}
	return false;
}

void InternalThread::Stop()
{
	isExit = true;
	emit StopExternalProcessing();
};

void InternalThread::run() 
{
	plugin.EmitStartSignal(id, type);

	if ( !plugin.IsUseExternalProcessing(inData.PluginSettings, lastErrorDesc) )
	{
		if ( type == SUPluginInterface::MakeMarking )
			res = plugin.GetMarking(id, inData, outData, lastErrorDesc);
		else if ( type == SUPluginInterface::BatchProcessing )
			res = BatchProcessing();
		else 
		{
			res = false;
			lastErrorDesc = QString::fromLocal8Bit("��� ����������.");
		}
	}
	else 
	{
		if ( res = plugin.PrepareForExernalProcessing(inData.PluginSettings,inData.PluginSettings,lastErrorDesc) )
		{
			usedExternalProcessing = true;
			ExternalProcessing(res);
		}
	}
}


bool InternalThread::BatchProcessing()
{
	int batchProcThreadCount;
	QList< QPair<QString, QString> > files;

	//! 1. ��������� ��������.
	if ( !plugin.GetCommonBatchSettings(inData.PluginSettings, files, batchProcThreadCount, lastErrorDesc) )
		return false;

	bool res = false;

	//! 2. 
	if( plugin.BeginBatchProcessing(id, inData.PluginSettings, lastErrorDesc) )
	{
		filesCount = files.size();
		QList< BatchThread * > threads;
		QList< QPair< QString, QString > >::iterator iter = files.begin();
		lastErrorDesc = "";

		//! 3. ������������ ����� � ��������� �������.
		while ( completedFilesCount < filesCount && !isExit )
		{
			//! �������� ����� �����, � ������� ����� ����.
			if ( threads.count() < batchProcThreadCount && iter != files.end() )
			{
				BatchThread * thread = new BatchThread(plugin);
				thread->Start(id, inData.PluginSettings);
				thread->SetFile(*iter);
				iter++;
				threads.append(thread);
			}
			else 
			{
				//! ��������� ��� ����� ��������� ����.
				for ( QList< BatchThread * >::iterator thIter = threads.begin(); 
						thIter != threads.end(); thIter++ )
				{
					if ( (*thIter)->ReadyForSet() )
					{
						completedFilesCount++;
						if ( iter != files.end() )
						{
							(*thIter)->SetFile(*iter);
							iter++;
						}
						break;
					}
					else if ( (*thIter)->isFinished() )
					{
						isExit = true;
						lastErrorDesc = (*thIter)->GetErrorDesc();
					}
				}
			}

			msleep(30);
		}

		//! 4. ��� ����������.
		for ( QList< BatchThread * >::iterator iter = threads.begin(); 
			iter != threads.end(); iter++ )
		{
			(*iter)->Stop();
			(*iter)->wait();
			delete (*iter);
		}


		//! 5. �������� ����������.
		res = plugin.GetCommonBatchResult();
	
		if ( lastErrorDesc.isEmpty() && !res)
			lastErrorDesc = QString::fromLocal8Bit("��������� ���������� �� ������ �����.");
	}

	plugin.EndBatchProcessing(id);

	return res;
}

bool InternalThread::GetFreePort(int & port, QString & lastErrorDesc)
{
	QTcpServer testServer;
	if ( !testServer.listen() )
	{
		lastErrorDesc = QString::fromLocal8Bit("��� ��������� ������!!!!");
		return false;
	}
	port = testServer.serverPort();
	return true;
}

Process * InternalThread::StartNewProcess(const QString & host, int & port, QString & lastErrorDesc)
{
	if ( GetFreePort(port, lastErrorDesc) )
	{
		Process * process = new Process;

		//!3. ������ ��������.
		if ( !process->start(processName, QStringList() << plugin.GetLibName() << QString::number(port) << host) )
		{
			lastErrorDesc = QString::fromLocal8Bit("��������� ��������� �������: \"%1\" , erorr: %2").arg(processName).arg(process->error());
			delete process;
		}
		else 
			return process;
	}
	return NULL;
}

PluginSocket * InternalThread::StartNewSocket(const QString & host, int port, QString & lastErrorDesc)
{
	PluginSocket * socket = new PluginSocket(*this);
	connect(this, SIGNAL(StopExternalProcessing()), socket, SLOT(StopExternalProcessing()), Qt::DirectConnection);
	connect(socket, SIGNAL(disconnected()), this, SLOT(OnDisconnected()), Qt::DirectConnection);

	if ( ConnectToServer(*socket, host, port, 60000) )
	{
		if ( type == SUPluginInterface::MakeMarking )
		{
			socket->StartGetMarking(id, inData);
			return socket;
		}
		else if ( type == SUPluginInterface::BatchProcessing )
		{
			
			QString settings;
			if ( CreateExternalBatchSettings(inData.PluginSettings, settings, lastErrorDesc) )
			{
				socket->StartProcessingFile(id, settings);
				return socket;
			}
		}
		else
			lastErrorDesc = QString::fromLocal8Bit("�������� �� ������������!!!!");
	
	}
	else
	{
		lastErrorDesc = QString::fromLocal8Bit("��������� ������������ � �������!!!!");
	}
	delete socket;
	return NULL;
}

bool InternalThread::CreateExternalBatchSettings(const QString & inSettings, QString & outSettings, QString & lastErrorDesc)
{
	try
	{
		json_spirit::mValue v;
		if ( json_spirit::read(inSettings.toUtf8().data(), v) )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();
			QString reportsDirPath = QString::fromUtf8(params["reportsDir"].get_str().data());
			QString processingDirPath = QString::fromUtf8(params["processingDir"].get_str().data());

			json_spirit::mArray * _array;
			//! ������������ ������ ������.
			if ( params.find("ProcessingFiles") != params.end() )
			{
				_array = &params["ProcessingFiles"].get_array();
				_array->clear();
			}
			else
			{
				params["ProcessingFiles"] = json_spirit::mArray();
				_array = &params["ProcessingFiles"].get_array();
			}

			//������ ������ ������.
			QStringList fileList = QDir(processingDirPath).entryList(QStringList()<< plugin.GetBatchFileProcessngMask(), QDir::Files);
			
			QString suffix = plugin.GetBatchFileResultMask();

			if ( !completedFiles.size() )
			{
				for ( QStringList::iterator iter = fileList.begin(); 
						iter != fileList.end(); iter++ )
					AddFilesDesc(*_array, *iter, suffix);
			}
			else 
			{
				QList< QString * > forProcessing;
				for ( QStringList::iterator iter = fileList.begin(); 
						iter != fileList.end(); iter++ )
				{
					if ( completedFiles.indexOf(*iter) == -1 )
						forProcessing.append(&(*iter));
				}

				for ( QList<QString *>::iterator iter = forProcessing.begin(); 
						iter != forProcessing.end(); iter++ )
					AddFilesDesc(*_array, *(*iter), suffix);
			}
			outSettings = QString::fromUtf8(json_spirit::write(v,json_spirit::pretty_print | json_spirit::raw_utf8).data());
			return true;

		}
		else 
			lastErrorDesc = QString::fromLocal8Bit("��������� ��������� ��������� JSON (CreateExternalBatchSettings)");
	}
	catch(std::exception & e)
	{
		lastErrorDesc = QString::fromLocal8Bit("������ �������� JSON (CreateExternalBatchSettings):").arg(e.what());
	}
	return false;
}

void InternalThread::AddFilesDesc(json_spirit::mArray & _array, const QString & fileName, const QString & suffix)
{
	QFileInfo info(fileName);
	QString fName = info.fileName();
	int sSize = info.suffix().size()+1;
	fName.remove(fName.size()  - sSize, sSize);
	fName += suffix;

	json_spirit::mObject obj;
	obj["file"] = json_spirit::mValue(fileName.toUtf8().data());
	obj["report"] = json_spirit::mValue(fName.toUtf8().data());

	_array.push_back(obj);
}


void InternalThread::ExternalProcessing(bool res)
{
	res = false;

	//1. �������� ������� ���� ���������.
	if ( enableCreateNewProcess )
	{
		if ( !(process = StartNewProcess(host, port, lastErrorDesc)) )
			return;
	}

	//2. �������� ������ � �������� ��������.
	if ( socket =  StartNewSocket(host, port, lastErrorDesc) )
		exec();

	std::cout << "event loop finished" << std::endl;


	//3. ������ ������.
	if ( socket )
	{
		delete socket;
		socket = NULL;
	}
	
	//4. ������ �������.
	if ( process )
	{
		process->kill();
		delete process;
		process = NULL;
	}
}


bool InternalThread::ConnectToServer(PluginSocket & socket, const QString & host, int port, int timeout )
{
	QTime time;
	time.start();

	int _timeout = timeout/3;

	//5. ���������� ������.
	while ( time.elapsed() < timeout )
	{
		socket.connectToHost(host, port, QIODevice::ReadWrite);

		if ( !socket.waitForConnected(_timeout) )
		{
			lastErrorDesc = socket.errorString();
		}
		else
			return true;
	}
	return false;
}

void InternalThread::ReceiveGetMarkingReply(bool _res, const QString & _lastErrorDesc, SUPOutData & _outData)
{
	outData = _outData;
	lastErrorDesc = _lastErrorDesc;
	res = _res;

	isExit = true;
	exit(1);
}

void InternalThread::ReceiveBatchProcessingReply(bool _res, const QString & _lastErrorDesc)
{
	lastErrorDesc = _lastErrorDesc;
	res = _res;

	isExit = true;
	exit(1);
}

void InternalThread::ReceiveEndFileProcessingReply(const SUPFileResult & fileResult)
{
	completedFiles.append(QFileInfo(fileResult.FileName).fileName());

	plugin.EmitFileProcessingResult(id, fileResult);
}

void InternalThread::ReceiveStartFileProcessingReply(const QString & fileName)
{
	plugin.EmitStartFileProcessing(id, fileName);
}

void InternalThread::ReceiveExitServer(bool result, const QString & resultDesc)
{
	//! �������� �����.
	isExit = true;
	exit(1);
}


void InternalThread::OnDisconnected()
{
	plugin.EmitProgressMessage(id, 0, QString::fromLocal8Bit("������ ���������� � ��������."));

	//! ������������� � ������ �������� ���������, ������� ��� ������� ����� ��������.
	if (!isExit && 
		type == SUPluginInterface::BatchProcessing && 
		enableCreateNewProcess && 
		curRestartCount < maxRestartCount )
	{
		socket->deleteLater();
		socket = NULL;

		//! ����� �������, 
		delete process;
		process = NULL;

		curRestartCount++;

		plugin.EmitProgressMessage(id, 0, QString::fromLocal8Bit("������� ����, ��������� �����!!"));
	
		//! ��������� ����� �������.
		if ( !((process = StartNewProcess(host, port, lastErrorDesc)) && 
			  (socket =  StartNewSocket(host, port, lastErrorDesc))) )
		{
			exit(1);
		}
	}
	else
		exit(1);
}