#ifndef BATCH_THREAD_H_ALEX
#define BATCH_THREAD_H_ALEX

#include <QThread>
#include <QPair>
#include <QMutex>
#include <QQueue>


class BaseSUPlugin;

//! Поток для пакетной обработки.
class BatchThread:public QThread
{
public:
	BatchThread(BaseSUPlugin & _plugin);
	
	void Start(int _id, const QString & _settings);

	void Stop() {stop = true;}
	
	bool SetFile(const QPair< QString, QString > & _data);

	bool ReadyForSet() const { return noData;};

	bool Result() const { return result; }

	QString GetErrorDesc() const { return errorDesc; }

protected:
	bool GetFile( QPair< QString, QString > & _data );
	
	void run();

private:
	BaseSUPlugin & plugin;
	QMutex mutex;
	QPair< QString, QString > data;
	bool noData;
	bool stop;
	int id;
	void * internalData;
	QString errorDesc;
	bool result;
	QString settings;
	//QQueue< QPair< QString, QString > > data;
};

#endif // BATCH_THREAD_H_ALEX
