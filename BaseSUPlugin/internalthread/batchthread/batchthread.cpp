#include "batchthread.h"
#include "../../basesuplugin.h"


BatchThread::BatchThread(BaseSUPlugin & _plugin):
	plugin(_plugin),
	noData(true),
	stop(false),
	id(0),
	internalData(NULL),
	result(false){}

void BatchThread::Start(int _id, const QString & _settings)
{
	Stop();
	wait();

	id = _id;
	settings = _settings;
	stop = false;
	result = false;
	errorDesc = "";
	start();
}

bool BatchThread::SetFile(const QPair< QString, QString > & _data)
{
	if ( noData )
	{
		QMutexLocker ml(&mutex);
		data = _data;
		noData = false;
	}
	return false;
}
bool BatchThread::GetFile( QPair< QString, QString > & _data )
{
	if ( !noData )
	{
		QMutexLocker ml(&mutex);
		_data = data;
		return true;
	}
	return false;
}

void BatchThread::run()
{
	if ( plugin.InitBatchProcessing(internalData, id, settings, errorDesc) )
	{

		QPair< QString, QString > files;
		while ( true )
		{
			if ( GetFile(files) )
			{
				plugin.FileProcessingInternal(internalData, id, files);
				noData = true;
			}
			else if ( stop )
				break;
			else 
				msleep(30);
		}
	}
	plugin.ReleaseBatchProcessing(internalData, id);
}
