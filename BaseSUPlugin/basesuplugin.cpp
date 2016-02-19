#include "basesuplugin.h"
#include <QThread>
#include <QDialog>
#include <QtPlugin>
#include <QtConcurrentRun>
#include <QFile>
#include <QDir>
#include "../Common/soundrecorder/soundrecorder.h"
#include <QMutex>
#include <QMenu>
#include <QToolBar>
#include <QLabel>
#include "../json_spirit/json_spirit.h"
#include <QUuid>
#include "internalthread/internalthread.h"



BaseSUPlugin::BaseSUPlugin():
	lastErrorDesc(QString::fromLocal8Bit("Инициализация не производилась.")),
	batchProcessingThread(new InternalThread(*this)),
	batchResultMutex(new QMutex())
{
	connect(batchProcessingThread, SIGNAL(finished()), this, SLOT(OnThreadFinished()));	
}

BaseSUPlugin::~BaseSUPlugin()
{
	foreach(InternalThread * thread, threads)
		delete thread;
	delete batchProcessingThread;
	delete batchResultMutex;
	Free();

}

//! Получить список результатов после пакетной обработки.
void BaseSUPlugin::GetBatchResult(QList<SUPFileResult> & result)
{
	QMutexLocker ml(batchResultMutex);
	result = batchResult;
}

bool BaseSUPlugin::GetCommonBatchResult()
{
	completedFiles.close();
	errorCompletedFiles.close();

	QMutexLocker ml(batchResultMutex);
	for ( QList<SUPFileResult>::iterator iter = batchResult.begin(); 
		iter != batchResult.end(); iter++ )
	{
		if ( iter->Result )
		{
			return true;
		}
	}
	return false;
}

InternalThread * BaseSUPlugin::GetFreeThread()
{
	if ( IsInit() )
	{
		InternalThread * curThread = NULL;

		int maxThreads = 32;

		int busyCount = 0;

		//Ищим поток.
		foreach(InternalThread * thread, threads)
		{
			if ( thread->IsCompleted() )
			{
				curThread = thread;
				break;
			}
			busyCount++;
		}

		//Не создаём потоков больше чем процессоров 
		if ( !curThread && busyCount < maxThreads )
		{
			curThread = new InternalThread(*this);
			connect(curThread, SIGNAL(finished()), this, SLOT(OnThreadFinished()));
			threads.append(curThread);
		}

		return curThread;
	}
	lastErrorDesc = QString::fromLocal8Bit("Нет свободных потоков.");
	return NULL;
}

void BaseSUPlugin::EmitStartSignal(int id, SUPluginInterface::ActionsType type)
{
	if ( type == SUPluginInterface::MakeMarking )
		emit MakeMarkingStarted(id);
	else if ( type == SUPluginInterface::BatchProcessing )
	{
		emit BatchProcessingStarted(id);

	}
}


bool BaseSUPlugin::StartMakeMarking(int id, const SUPInputData & inData)
{
	InternalThread * curThread = GetFreeThread();
	if ( curThread )
		return curThread->Start(id, inData, SUPluginInterface::MakeMarking);
	return false;
}



void BaseSUPlugin::OnThreadFinished()
{
	InternalThread * thread = (InternalThread *)sender();
	thread->SetCompleted();

	if ( thread->GetType() == SUPluginInterface::MakeMarking )
	{
		QString str = thread->GetLastErrorDesc();
		bool res = PostProcessResult(thread->GetID(), thread->GetOutData(), thread->GetResult(), str);
		emit MakeMarkingFinished(thread->GetID(), thread->GetOutData(), res, str);
	}
	else if ( thread->GetType() == SUPluginInterface::BatchProcessing )
	{
		emit BatchProcessingFinished(thread->GetID(), thread->GetResult(), thread->GetLastErrorDesc());
	}
}

bool BaseSUPlugin::PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc)
{
	return result;
}

bool BaseSUPlugin::BeginBatchProcessing(int id, const QString & settings, QString & errorDesc)
{
	return true;
}

void BaseSUPlugin::EndBatchProcessing(int id)
{

}



bool BaseSUPlugin::StartBatchProcessing(int id, const QString & settings)
{
	if ( batchProcessingThread->IsCompleted() )
	{
		SUPInputData inData;
		inData.PluginSettings = settings;
		batchResult.clear();
		return batchProcessingThread->Start(id, inData, SUPluginInterface::BatchProcessing);
	}
	lastErrorDesc = QString::fromLocal8Bit("Выполняется пакетная обработка.");
	return false;
}

bool BaseSUPlugin::StopBatchProcessing(int id)
{
	batchProcessingThread->Stop();
	return true;
}
bool BaseSUPlugin::GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc)
{
	errorDesc = "Plugin not implemented!!!";
	return false;
}


bool BaseSUPlugin::GetCommonBatchSettings(const QString & settings,
										QList< QPair<QString, QString> > & outList,
										int & maxBPThreadCount,
										QString & errorDesc)
{
	try 
	{
		json_spirit::mValue v;
		if ( json_spirit::read(settings.toUtf8().data(), v) )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();
			QString reportsDirPath = QString::fromUtf8(params["reportsDir"].get_str().data());
			QString processingDirPath = QString::fromUtf8(params["processingDir"].get_str().data());
			maxBPThreadCount = params["maxBPThreadCount"].get_int();
			maxBPThreadCount = maxBPThreadCount < 1 ? 1 : maxBPThreadCount;

			//! Используется список файлов.
			if ( params.find("ProcessingFiles") != params.end() )
			{
				json_spirit::mArray & _array = params["ProcessingFiles"].get_array();
				outList.clear();
				
				for ( json_spirit::mArray::iterator iter = _array.begin();
					iter != _array.end(); iter++ )
				{
					outList.append( QPair< QString, QString >(processingDirPath + "/" + QString::fromUtf8(iter->get_obj()["file"].get_str().data()), 
															reportsDirPath + "/" + QString::fromUtf8(iter->get_obj()["report"].get_str().data())) );
				}
				return outList.size();
			}
			else
			{
				//! Файлы на обработку
				QDir filesDir(processingDirPath);
				QStringList files;

				if( filesDir.exists() )
				{
					files = filesDir.entryList(QStringList()<< GetBatchFileProcessngMask(), QDir::Files);
					if ( !files.size() )
					{
						errorDesc = QString::fromLocal8Bit("Неудалось загрузить список фалов из: \"%1\"").arg(processingDirPath);
						return false;
					}

					//Отбросим часть файлов.
					if ( params.find("restartProcessing") != params.end() && 
						params["restartProcessing"].get_bool() == false )
					{
						QFile file(GetPluginName() + ".completed");
						if ( file.open(QFile::ReadOnly) )
						{
							QTextStream stream(&file);
							stream.setCodec("UTF-8");
							QString line;
							int index;

							//Удалим все обработанные файлы.
							do
							{
								line = stream.readLine();
								index = files.indexOf(line);
								if( index != -1 )
									files.removeAt(index);
							}
							while (!line.isNull());
						}
						file.close();

						file.setFileName(GetPluginName() + ".errorCompleted");
						if ( file.open(QFile::ReadOnly) )
						{
							QTextStream stream(&file);
							stream.setCodec("UTF-8");
							QString line;
							int index;

							//Удалим все обработанные файлы c ошибкой.
							do
							{
								line = stream.readLine();
								index = files.indexOf(line);
								if( index != -1 )
									files.removeAt(index);
							}
							while (!line.isNull());
						}
						file.close();
					}
					//Создадим файл обработанных файлов.
					completedFiles.setFileName(GetPluginName() + ".completed");
					errorCompletedFiles.setFileName(GetPluginName() + ".errorCompleted");
					
					if ( params.find("restartProcessing") != params.end() && 
							params["restartProcessing"].get_bool() == false )
					{
						completedFiles.open(QFile::WriteOnly | QIODevice::Append);
						errorCompletedFiles.open(QFile::WriteOnly | QIODevice::Append);
					}
					else
					{
						completedFiles.open(QFile::WriteOnly);
						errorCompletedFiles.open(QFile::WriteOnly);
					}


					completedFilesStream.setDevice(&completedFiles);
					completedFilesStream.setCodec("UTF-8");

					errorCompletedFilesStream.setDevice(&errorCompletedFiles);
					errorCompletedFilesStream.setCodec("UTF-8");
				}
				else 
				{
					errorDesc = QString::fromLocal8Bit("Неудалось загрузить список фалов из: \"%1\"").arg(processingDirPath);
					return false;
				}

				//Создадим директорию отчётов.
				QDir reportDir(reportsDirPath);
				if ( !reportDir.exists() && !reportDir.mkpath(".") )
				{
					errorDesc = QString::fromLocal8Bit("Неудалось создать директорию отчётов: \"%1\"").arg(reportsDirPath);
					return false;
				}

				outList.clear();
				for ( QStringList::iterator iter = files.begin(); iter != files.end(); iter++ )
				{
					QFileInfo info(*iter);
					QString fName = info.fileName();
					int sSize = info.suffix().size()+1;
					fName.remove(fName.size()  - sSize, sSize);

					outList.append(QPair< QString, QString >(filesDir.absolutePath() + "/" + *iter,
						reportDir.absolutePath() + "/" + fName + GetBatchFileResultMask()));
				}

				return true;
			}
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек пакетной обработки: %1").arg(e.what());
	}
	return false;
}

bool BaseSUPlugin::IsUseExternalProcessing(const QString & settings, QString & errorDesc)
{
	try 
	{
		json_spirit::mValue v;
		if ( json_spirit::read(settings.toUtf8().data(), v) )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();
			return params["useExternalProcessing"].get_bool();
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек для useExternalProcessing: %1").arg(e.what());
	}
	return false;
}

bool BaseSUPlugin::PrepareForExernalProcessing(const QString & settings, 
												QString & outSettings, 
												QString & errorDesc)
{
	try 
	{
		json_spirit::mValue v;
		if ( json_spirit::read(settings.toUtf8().data(), v) )
		{
			json_spirit::mObject & params = v.get_obj()["parameters"].get_obj();
			params["useExternalProcessing"] = false;
			outSettings = QString::fromUtf8(json_spirit::write(v,
											json_spirit::pretty_print | json_spirit::raw_utf8).data());
			return true;
		}
	}
	catch( std::exception & e)
	{
		errorDesc  = QString::fromLocal8Bit("Ошибка парсинга настроек для useExternalProcessing: %1").arg(e.what());
	}
	return false;
}


void BaseSUPlugin::FileProcessingInternal(void * internalData, int id, const QPair<QString, QString> & file)
{
	SUPFileResult res;
	res.FileName = file.first;
	
	EmitStartFileProcessing(id, file.first);
	res.Result = FileProcessing(internalData, id, file, res.ErrorDesc);
	EmitFileProcessingResult(id, res);

	//! Добавим результат обработки.
	batchResultMutex->lock();
	batchResult.append(res);

	if ( res.Result )
	{
		completedFilesStream << QFileInfo(file.first).fileName() << '\n';
		completedFilesStream.flush();
	}
	else
	{
		errorCompletedFilesStream << QFileInfo(file.first).fileName() << '\n';
		errorCompletedFilesStream.flush();
	}
	batchResultMutex->unlock();
}

void BaseSUPlugin::EmitFileProcessingResult(int id, const SUPFileResult & res)
{
	emit FileProcessingResult(id, res);
}


void BaseSUPlugin::EmitProgressMessage(int id, qreal progress, const QString & message)
{
	emit ProgressMessage(id, progress, message);
}

void BaseSUPlugin::EmitStartFileProcessing(int id, const QString & fileName)
{
	emit StartFileProcessing(id, fileName);
}

int BaseSUPlugin::GetCompletedFilesCount() const
{ 
	return batchProcessingThread->GetCompletedFilesCount();
}

int BaseSUPlugin::GetFilesCount() const 
{
	return batchProcessingThread->GetFilesCount();
}

bool BaseSUPlugin::InitBatchProcessing(void * & internalData, int id, const QString & settings, QString & errorDesc)
{
	errorDesc = "Plugin not implemented!!!";
	return false;
}

bool BaseSUPlugin::FileProcessing(void * internalData, int id, const QPair<QString, QString> & file_report, QString & errorDesc)
{
	errorDesc = "Plugin not implemented!!!";
	return false;
}
void BaseSUPlugin::ReleaseBatchProcessing(void * & internalData, int id)
{
}

bool BaseSUPlugin::SaveMarkingInFile(const QString & fileName, const QList<SUPPhraseData> & marking, QString & lastErrorDesc)
{
	//Сохраним разметку.
	QFile file(fileName);
	if ( file.open(QIODevice::WriteOnly) )
	{
		QTextStream out(&file);

		for ( QList<SUPPhraseData>::const_iterator iter = marking.constBegin();
				iter != marking.constEnd(); iter++ )
			out << (quint64)(iter->Start / 8000. * 1e7) << " " << (quint64)(iter->End / 8000. * 1e7) << " " << iter->Name << "\n";
		return true;
	}
	else
		lastErrorDesc = QString::fromLocal8Bit("Неудалось создать файл разметки: \"%1\"").arg(fileName);
	return false;
}


QMenu * BaseSUPlugin::InitActions(QMenu * inMenu, QToolBar * inToolBar,
								  unsigned int actionsMask)
{
	if ( inMenu && inToolBar )
	{
		QMenu * menu = new QMenu(GetPluginName());
		QObject * mainParent = inMenu->parent();
		QList< QAction *> actonsList;

		QAction * action;
		if ( actionsMask & 0x1 )
		{
			action = new QAction(QPixmap(":/img/Resources/settings.png"), QString::fromLocal8Bit("Настройка"), mainParent);
			menu->addAction(action);
			connect(action, SIGNAL(triggered(bool)), this, SLOT(OpenSettingsDialog()));
			inToolBar->addWidget(new QLabel(GetPluginName()));
			inToolBar->addAction(action);
			actonsList.push_back(action);
		}

		if ( actionsMask & 0x2 )
		{
			action = new QAction(QPixmap(":/img/Resources/marking.png"), QString::fromLocal8Bit("Проанализировать"), mainParent);
			menu->addAction(action);
			connect(action, SIGNAL(triggered(bool)), this, SIGNAL(GetMarkingTriggered()));
			inToolBar->addAction(action);
			actonsList.push_back(action);
		}

		if ( actionsMask & 0x4 )
		{
			action = new QAction(QPixmap(":/img/Resources/marking_by_marking.png"), QString::fromLocal8Bit("Проанализировать на базе разметки"), mainParent);
			menu->addAction(action);
			connect(action, SIGNAL(triggered(bool)), this, SIGNAL(GetMarkingOnMarkingTriggered()));
			inToolBar->addAction(action);
			actonsList.push_back(action);
		}

		if ( actionsMask & 0x8 )
		{
			action = new QAction(QPixmap(":/img/Resources/start.png"),QString::fromLocal8Bit("Запустить пакетную обработку"), mainParent);
			menu->addAction(action);
			connect(action, SIGNAL(triggered(bool)), this, SIGNAL(StartBatchProcessingTriggered()));
			inToolBar->addAction(action);
			actonsList.push_back(action);
		}

		if ( actionsMask & 0x10 )
		{
			action = new QAction(QPixmap(":/img/Resources/stop.png"),QString::fromLocal8Bit("Остановить пакетную обработку"), mainParent);
			menu->addAction(action);
			connect(action, SIGNAL(triggered(bool)), this, SIGNAL(StopBatchProcessingTriggered()));
			inToolBar->addAction(action);
			inMenu->addMenu(menu);
			actonsList.push_back(action);
		}

		RenameActionsNames(actonsList);

		return menu;
	}
	return NULL;
}

void BaseSUPlugin::RenameActionsNames(QList< QAction *> & actions)
{

}

bool BaseSUPlugin::LoadFile(const QString & fileName, int bitPerSample, int samplerate, QByteArray & signal, QString & lastErrorDesc)
{
	QFile file(fileName);
	SoundRecorder::WaveFormat soundFormat;

	if ( !SoundRecorder::ReadWaveFile(&file, soundFormat, signal))
	{
		lastErrorDesc = QString::fromLocal8Bit("Неудалось прочитать файл: \"%1\"").arg(fileName);
		return false;
	}

	if ( soundFormat.bitsPerSample != bitPerSample || soundFormat.sampleRate != samplerate )
	{
		lastErrorDesc = QString::fromLocal8Bit("Неудалось прочитать файл: \"%1\" bitsPerSample: %2 sampleRate: %3").arg(fileName).
			arg(soundFormat.bitsPerSample).arg(soundFormat.sampleRate);
		return false;
	}
	return true;
}

bool BaseSUPlugin::SaveFile(const QString & fileName, int channels, int bitPerSample, int samplerate, QByteArray & signal, QString & lastErrorDesc)
{
	QFile file(fileName);

	if ( !SoundRecorder::WriteWaveFile(&file, channels, samplerate, bitPerSample,  signal))
	{
		lastErrorDesc = QString::fromLocal8Bit("Неудалось записать данные в файл: \"%1\"").arg(fileName);
		return false;
	}
	return true;
}


//! Опрераторы для сериализации.
QDataStream & operator << (QDataStream & out, const SUPPhraseData & data)
{
	out << data.Start;
	out << data.End;	
	out << data.Name;
	return out;
}

//! Опрераторы для сериализации.
QDataStream & operator>>(QDataStream & out, SUPPhraseData & data)
{
	out >> data.Start;
	out >> data.End;	
	out >> data.Name;
	return out;
}

QDataStream & operator << (QDataStream & out, const SUPInputData & data)
{
	out << data.BitsPerSample;
	out << data.SampleRate;
	out << data.Signal;
	out << data.Intervals;
	out << data.PluginSettings;
	return out;
}

QDataStream & operator >> (QDataStream & out, SUPInputData & data)
{
	out >> data.BitsPerSample;
	out >> data.SampleRate;
	out >> data.Signal;
	out >> data.Intervals;
	out >> data.PluginSettings;
	return out;
}

QDataStream & operator << (QDataStream & out, const SUPOutData & data)
{
	out << data.Marking;
	out << data.AdditionData;
	return out;
}

QDataStream & operator >> (QDataStream & out, SUPOutData & data)
{
	out >> data.Marking;
	out >> data.AdditionData;
	return out;
}


QDataStream & operator << (QDataStream & out, const SUPFileResult & data)
{
	out << data.FileName;
	out << data.ErrorDesc;
	out << data.Result;
	return out;
}

QDataStream & operator >> (QDataStream & out, SUPFileResult & data)
{
	out >> data.FileName;
	out >> data.ErrorDesc;
	out >> data.Result;
	return out;
}