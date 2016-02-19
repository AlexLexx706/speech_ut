#include "mainwindow.h"
#include <QFileDialog>
#include <QBuffer>
#include <QMessageBox>
#include <iostream>
#include "waveviewerwidget/waveviewerwidget.h"
#include "phrasecontroller/phrasecontroller.h"
#include "phrasecontroller/phrase/phrase.h"
#include "phrasecontroller/phrasesList/phraseslist.h"
#include "phrasecontroller/phrasesList/phraseslistdata/phraseslistdata.h"
#include "phrasecontroller\phrasecontroller.h"
#include <QColorDialog>
#include "searchdialog/searchdialog.h"
#include "gradientdialog/gradientdialog.h"
#include "waveviewerwidget/frame/dataframe/dataframe.h"
#include "fileview/fileview.h"
#include <math.h>
#include <QMetaMethod>
#include <QPluginLoader>
#include <QTime>
#include "settingsdialog/settingsdialog.h"
#include <QCoreApplication>
#include "../json_spirit/json_spirit.h"
#include "removephrasesbynamedialog/removephrasesbynamedialog.h"
#include "ui_mainwindow.h"
#include "controlsframe/extcontrolsframe/extcontrolsframe.h"
#include "controlsframe/simplecontrolsframe/simplecontrolsframe.h"
#include <QDomDocument>
#include <QTextStream>
#include "aboutdialog/aboutdialog.h"
#include <QDebug>
#include "signaldescframe/signaldescframe.h"
#include "OpenedFilesDialog/openedfilesdialog.h"
#include <QFileInfo>

QVariant JsonValueToVariant(const json_spirit::mValue & v);

struct ssss{
	int i;
};
void Test(const std::vector<ssss *> & pp)
{
	pp[0]->i = 10;
}

#define ui (*((Ui::MainWindow *)_ui))
MainWindow::MainWindow(const QVariantMap & in_settings,
						QWidget *parent): 
	QMainWindow(parent),
	audioOutput(NULL),
	offset(0),
	settings("SOFTWARE\\Stel Computer Systems\\speech_utility", QSettings::NativeFormat),
	pluginProgressCount(0),
	pluginTaskCounter(0),
	messageCounter(1),
	curMode(MainWindow::Normal),
	minSpeechDur(20.f),
	useDictorCount(false),
	controlsFrame(NULL),
	makeXml(false),
	logFile("pluginsLog.txt"),
	signal_desc_frame(new SignalDescFrame(this))
{
	logFile.open(QIODevice::WriteOnly);
	logStream.setDevice(&logFile);
	logStream.setCodec("UTF-8");
	QStringList inputFiles;

	//Настройки
	if ( in_settings["usignMode"].isValid() )
		curMode = (UsignMode)in_settings["usignMode"].toInt();
	
	if ( in_settings["inputFiles"].isValid() )
	{
		QVariantList vList = in_settings["inputFiles"].toList();
		foreach(const QVariant & v, vList)
			inputFiles << v.toString();
	}
	
	if ( in_settings["outFile"].isValid() )
		outFileName = in_settings["outFile"].toString();
	
	if ( in_settings["minSpeechDur"].isValid() )
		minSpeechDur = in_settings["minSpeechDur"].toReal();

	if ( in_settings["useDictorCount"].isValid() )
		useDictorCount = in_settings["useDictorCount"].toBool();

	if ( in_settings["makeXml"].isValid() )
		makeXml = in_settings["makeXml"].toBool();

	_ui = new Ui::MainWindow;
	qRegisterMetaType<SUPPhraseData>("SUPPhraseData");
	qRegisterMetaType<QList<SUPPhraseData>>("QList<SUPPhraseData>");
	qRegisterMetaType<SUPFileResult>("SUPFileResult");
	qRegisterMetaType<SUPInputData>("SUPInputData");
	qRegisterMetaType<SUPOutData>("SUPOutData");

	ui.setupUi(this);

	LoadPlugins("");
	SetupControlsFrame(new ExtControlsFrame(this));
	//SetupControlsFrame(new SimpleControlsFrame(this));
	settingsDialog = new SettingsDialog(this);
	ui.toolBar_signal_desc->addWidget(signal_desc_frame);
	signal_desc_frame->setEnabled(false);
	ui.toolBar_signal_desc->setEnabled(true);
	connect(signal_desc_frame, SIGNAL(DataChanged()), this, SLOT(OnSignalDescChanged()));
	ui.toolBar_signal_desc->setVisible(settingsDialog->IsShowSignalDescInStartup());

	ui.tabWidget->insertAction(NULL, ui.action_closeFile);
	ui.tabWidget->insertAction(NULL, ui.action_closeAllFiles);
	ui.toolBar_buttons->addWidget(ui.frame_buttons);

	ui.frame_pluginDebug->hide();
	ui.progressBar_pluginDebug->hide();

	searchDialog = new SearchDialog(this);
	searchDialog->hide();
	connect(searchDialog, SIGNAL(FindNext()), this, SLOT(OnFindNext()));
	

	ui.tabWidget->ClearAll();
	ui.tabWidget->SetPhrasesListMenu(ui.menu_marking);
	ui.tabWidget->SetMenuEdit(ui.menu_edit);
	ui.tabWidget->SetMenuView(ui.menu_formats);
	ui.plainTextEdit_pluginDebug->addAction(ui.action_hidePluginFrame);
	ui.plainTextEdit_pluginDebug->addAction(ui.action_clearPluginMessages);
	ui.plainTextEdit_pluginDebug->addAction(ui.action_copyPluginMessages);

	connect( ui.tabWidget, SIGNAL(SelectedSamplesChanged(long, long)), SLOT(OnSelectedSamplesChanged(long, long)) );
	connect( ui.tabWidget, SIGNAL(PosChanged(long, double)), SLOT(OnPosChanged(long, double)) );
	connect( ui.tabWidget, SIGNAL(CurrentPhraseChanged()), SLOT(OnCurrentPhraseChanged()) );
	connect( ui.tabWidget, SIGNAL(PhraseDataChanged()), SLOT(OnPhraseDataChanged()) );
	connect( ui.tabWidget, SIGNAL(CurrentListChanged()), SLOT(OnCurrentListChanged()) );
	connect( ui.tabWidget, SIGNAL(ListDataChanged()), SLOT(OnCurrentListChanged()) );

	connect( ui.tabWidget, SIGNAL(DataChanged(long)), this, SLOT(onDataChanged(long)));

	connect(ui.tabWidget, SIGNAL(DropWavFile(const QString &)), this, SLOT(OnDropWavFile(const QString &)));
	connect(ui.tabWidget, SIGNAL(DropMarkingFile(const QString &)), this, SLOT(OnDropMarkingFile(const QString &)));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnCurViewChanged()));

	connect(ui.radioButton_wave, SIGNAL(clicked(bool)), this, SLOT(OnChangeType()) );
	connect(ui.radioButton_spector, SIGNAL(clicked(bool)), this, SLOT(OnChangeType()) );


	OnCurViewChanged();

	//! Настройка.
	ui.toolButton_selectSamples->setChecked(true);
	controlsFrame->SetEnabledPlayback(false);
	ui.toolButton_selectSamples->hide();

	//! Нормальный режим работы	
	if ( curMode == MainWindow::Normal )
	{
		if (inputFiles.isEmpty())
			OpenFile(settings.value("lastFileName", "").toString());
		else
		{
			foreach(const QString & file_path, inputFiles)
				OpenFile(file_path);
		}

		if ( in_settings["startPacketVad"].isValid() && in_settings["startPacketVad"].toBool())
		{
			//Передадим дополнительное описание результата.
			if ( SUPluginInterface * plugin = GetPluginByName("Plugin_VAD") )
			{
				if ( plugin->StartBatchProcessing(pluginTaskCounter, plugin->GetPluginSettings()) )
					pluginTaskCounter++;
			}
			else 
				std::cout << "Plugin_VAD not exist!!!" << std::endl;
		}
	}
	else
	{
		OpenFilesAsOne(inputFiles);

		//Передадим дополнительное описание результата.
		if ( SUPluginInterface * plugin = GetPluginByName("Plugin_VAD") )
		{
			try 
			{
				json_spirit::mValue value;
				
				if ( json_spirit::read(plugin->GetPluginSettings().toUtf8().data(), value) )
				{
					json_spirit::mValue v;
					json_spirit::read(value.get_obj()["parameters"].get_obj()["vadParams"].get_str(), v);
					v.get_obj()["vadModels"].get_array()[0].get_obj()["useParams2"] = true;
					value.get_obj()["parameters"].get_obj()["vadParams"] = json_spirit::write(v,json_spirit::pretty_print | json_spirit::raw_utf8);
					value.get_obj()["parameters"].get_obj()["UseDictorCount"] = useDictorCount;
					
					if ( !StartMakeMarking(plugin, QString::fromUtf8(json_spirit::write(value,json_spirit::pretty_print | json_spirit::raw_utf8).data())) )
					{
						std::cout << "Canot Start Plugin_VAD!!!" << std::endl;
						exit(1);
					}
				}
				else
				{
					std::cout << "Cannot set Plugin_VAD!!!" << std::endl;
					exit(1);
				}
			}
			catch( std::exception & e )
			{
				std::cout << "cannot parse JSON params of Plugin_VAD, error: " << e.what() << std::endl;
			}

		}
		else 
		{
			std::cout << "Plugin_VAD not exist!!!" << std::endl;
			exit(1);
		}
	}

	//SetUsignMode(UsignMode::ExtEdit);
}

void MainWindow::onDataChanged(long)
{
	Stop();
}

void MainWindow::SetupControlsFrame(ControlsFrame * controls)
{
	if ( controls )
	{
		delete controlsFrame;
		controlsFrame = controls;
		ui.toolBar_controlls->addWidget(controlsFrame);

		connect(controlsFrame, SIGNAL(StartSelectedChanged(long)), this, SLOT(onStartSelectedChanged(long)));
		connect(controlsFrame, SIGNAL(EndSelectedChanged(long)), this, SLOT(onEndSelectedChanged(long)));
		connect(controlsFrame, SIGNAL(MarkingNameChanged(const QString &)), this, SLOT(onMarkingNameChanged(const QString &)));
		connect(controlsFrame, SIGNAL(MarkingIntersectionChanged(bool)), this, SLOT(onMarkingIntersectionChanged(bool)));
		connect(controlsFrame, SIGNAL(MarkingColorChanged(const QColor &)), this, SLOT(onMarkingColorChanged(const QColor &)));
		connect(controlsFrame, SIGNAL(MoveMarkingUp()), this, SLOT(onMoveMarkingUp()));
		connect(controlsFrame, SIGNAL(MoveMarkingDown()), this, SLOT(onMoveMarkingDown()));
		connect(controlsFrame, SIGNAL(StartPhraseChanged(long)), this, SLOT(onStartPhraseChanged(long)));
		connect(controlsFrame, SIGNAL(EndPhraseChanged(long)), this, SLOT(onEndPhraseChanged(long)));
		connect(controlsFrame, SIGNAL(PhraseNameChanged(const QString &)), this, SLOT(onPhraseNameChanged(const QString &)));
		connect(controlsFrame, SIGNAL(PlaybackChanged(bool)), this, SLOT(onPlaybackChanged(bool)));
		connect(controlsFrame, SIGNAL(PauseClicked()), this, SLOT(onPauseClicked()));
	}
}

SUPluginInterface * MainWindow::GetPluginByName(const QString & name)
{
	for ( QList< QPair< SUPluginInterface *, QPluginLoader * >  >::iterator iter = pluginsList.begin();
		iter != pluginsList.end(); iter++ )
	{
		if ( iter->first->GetPluginName() == name )
			return iter->first;
	}
	return false;
}

void MainWindow::GetAllActions(QList<QAction *> & list)
{
	list = findChildren<QAction *>();
}

void MainWindow::GetAllButtons(QList<QAbstractButton *> & list)
{
	list = findChildren<QAbstractButton *>();
}

MainWindow::~MainWindow()
{
	RemoveAllPlugins();
	delete settingsDialog;
	delete (Ui::MainWindow *)_ui;
}

FileViewController &  MainWindow::GetFVC()
{
	return *ui.tabWidget;
}


QString MainWindow::FromSecToStrign(qreal t)
{
	if (t >= 0) {
		qreal ip;
		qreal fp = modf(t, &ip);
		long sec = ip;
		QString res = QString("%1").arg(fp, 0, 'f', 1).mid(2, 2);
		long s = sec % 60;
		sec = sec - s;
		long m = sec / 60;
		return QString("%1:%2.%3").arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0')).arg(res);
	}
	return "00:00.0";
}

bool MainWindow::FromStringToSec(const QString & time, qreal & res)
{
	//1 минуты.
	QRegExp reg("^(\\d*:)?(\\d?\\d?.?)(\\d*)?");
	if ( reg.indexIn(time) > -1 )
	{
		long min = reg.cap(1).remove(":").toLong() * 60;
		long sec = reg.cap(2).remove(".").toLong();
		QString dSec= QString("0.%1").arg(reg.cap(3));
		res = (qreal)(min + sec) + dSec.toDouble();
		return true;
	}
	return false;
}

void MainWindow::LoadPlugins(const QString & path)
{
	RemoveAllPlugins();

	QDir dir(path);
	QStringList files = dir.entryList(QStringList() << "*Plugin.dll", QDir::Files);

	foreach(QString file, files)
	{
	QPluginLoader * pluginLoader = new QPluginLoader(dir.absoluteFilePath(file));
	if ( QObject * obj = pluginLoader->instance() ) 
	{
		if ( SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(obj) )
		{
			QToolBar * tb = new QToolBar(plugin->GetPluginName(), this);
			tb->setToolTip(plugin->GetPluginName());
			if ( plugin->Init(&settings, ui.menu_marking, tb) )
			{
				addToolBar(Qt::TopToolBarArea, tb);

				connect(obj, SIGNAL(GetMarkingTriggered()), this, SLOT(OnGetMarkingTriggered()));
				connect(obj, SIGNAL(GetMarkingOnMarkingTriggered()), this, SLOT(OnGetMarkingOnMarkingTriggered()));
				connect(obj, SIGNAL(StartBatchProcessingTriggered()), this, SLOT(OnStartBatchProcessingTriggered()));
				connect(obj, SIGNAL(StopBatchProcessingTriggered()), this, SLOT(OnStopBatchProcessingTriggered()));

				connect(obj, SIGNAL(MakeMarkingStarted(int)), this, SLOT(OnMakeMarkingStarted(int)));
				connect(obj, SIGNAL(MakeMarkingFinished(int, const SUPOutData &, bool, const QString &)), this, 
					SLOT(OnMakeMarkingFinished(int, const SUPOutData &, bool, const QString &)));

				connect(obj, SIGNAL(BatchProcessingStarted(int)), this, SLOT(OnBatchProcessingStarted(int)));
				connect(obj, SIGNAL(BatchProcessingFinished(int, bool, const QString &)), this, SLOT(OnBatchProcessingFinished(int, bool, const QString &)));
				connect(obj, SIGNAL(ProgressMessage(int, qreal, const QString &)), this, SLOT(OnProgeressMessage(int, qreal, const QString &)));
				connect(obj, SIGNAL(FileProcessingResult(int, const SUPFileResult &)), this, SLOT(OnFileProcessingResult(int, const SUPFileResult &)));
				connect(obj, SIGNAL(StartFileProcessing(int, const QString &)), this, SLOT(OnStartFileProcessing(int, const QString &)));

				pluginsList.append( QPair< SUPluginInterface *, QPluginLoader * >(plugin, pluginLoader) );
				continue;
			}
			else 
				OnProgeressMessage(0, 0, QString::fromUtf8("Ошибка инициализации плагина: %1 %2.").arg(plugin->GetPluginName()).arg(plugin->GetLastErrorDesc()));
			delete tb;
		}
		delete obj;
	}
	delete pluginLoader;
	}
}

void MainWindow::RemoveAllPlugins()
{
	for (QList< QPair< SUPluginInterface *, QPluginLoader * > >::iterator iter = pluginsList.begin();
		iter != pluginsList.end(); iter++ )
	{
		delete iter->second;
	}
	pluginsList.clear();
}

void MainWindow::OnGetMarkingTriggered()
{
	if ( SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender()) )
		StartMakeMarking(plugin, plugin->GetPluginSettings());
}

bool MainWindow::StartMakeMarking(SUPluginInterface * plugin, const QString & params)
{
	if (!plugin )
		return false;

	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		QPair<long, long> samples = view->GetWaveWidget()->GetSelectedSamples();
		DataFrame * dataFame = view->GetWaveWidget()->GetWaveData();
		samples.second = samples.first == samples.second ? dataFame->GetCount() : samples.second;

		SUPInputData inputData;
		inputData.BitsPerSample = dataFame->GetBitPersample();
		inputData.SampleRate = dataFame->GetSampleRate();
		inputData.Signal = dataFame->GetData();
		inputData.Intervals.append(QPair<qint32,qint32>(samples.first, samples.second));
		inputData.PluginSettings = params;

		TempPluginData tempData;
		tempData.View = view;

		if (plugin->StartMakeMarking(pluginTaskCounter, inputData) )
		{
			tempPluginDataInt.insert(pluginTaskCounter, tempData);
			pluginTaskCounter++;
			return true;
		}
		else 
			OnProgeressMessage(0, 0, QString::fromUtf8("Ошибка запуска плагин: %1 %2.").arg(plugin->GetPluginName()).arg(plugin->GetLastErrorDesc()));
	}
	return false;
}


void MainWindow::OnGetMarkingOnMarkingTriggered()
{
	if ( SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender()) )
		StartMakeMarkingOnMarking(plugin, plugin->GetPluginSettings());
}


bool MainWindow::StartMakeMarkingOnMarking(SUPluginInterface * plugin, const QString & params)
{
	if ( !plugin )
		return false;

	if ( FileView * view = ui.tabWidget->GetCurFileView()  )
	{
		if ( PhrasesListData * list = view->GetPhraseController()->GetCurPhraseListData() )
		{
			QPair<long, long> samples = view->GetWaveWidget()->GetSelectedSamples();
			DataFrame * dataFame = view->GetWaveWidget()->GetWaveData();

			SUPInputData inputData;
			inputData.BitsPerSample = dataFame->GetBitPersample();
			inputData.SampleRate = dataFame->GetSampleRate();
			inputData.Signal = dataFame->GetData();
			inputData.PluginSettings = params;

			for ( QList<Phrase * >::const_iterator iter = list->GetPhrases().constBegin(); 
				iter != list->GetPhrases().constEnd(); iter++ )
			{
				inputData.Intervals.append(QPair<qint32, qint32>((*iter)->GetStart(), (*iter)->GetEnd()));
			}

			TempPluginData tempData;
			tempData.View = view;

			if (plugin->StartMakeMarking(pluginTaskCounter, inputData) )
			{
				tempPluginDataInt.insert(pluginTaskCounter, tempData);
				pluginTaskCounter++;
				return true;
			}
			else 
				OnProgeressMessage(0, 0, QString::fromUtf8("Ошибка запуска плагин: %1 %2.").arg(plugin->GetPluginName()).arg(plugin->GetLastErrorDesc()));
		}
	}
	return false;
}


void MainWindow::OnStartBatchProcessingTriggered()
{
	if ( SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender()) )
	{
		if ( plugin->StartBatchProcessing(pluginTaskCounter, plugin->GetPluginSettings()) )
			pluginTaskCounter++;
		else 
			OnProgeressMessage(0, 0, QString::fromUtf8("Ошибка запуска плагин: %1 %2.").arg(plugin->GetPluginName()).arg(plugin->GetLastErrorDesc()));
	}
}

void MainWindow::OnStopBatchProcessingTriggered()
{
	SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender());
	plugin->StopBatchProcessing(0);
}

void MainWindow::OnMakeMarkingStarted(int id)
{
	if ( settingsDialog->ShowPluginMassages() )
		on_action_showPluginFrame_triggered(true);
	ui.progressBar_pluginDebug->show();
	pluginProgressCount++;

	SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender());
	OnProgeressMessage(id, 0, QString::fromUtf8("Получение разметки плагина: %1 запущено.").arg(plugin->GetPluginName()));

}

void MainWindow::OnMakeMarkingFinished(int id, const SUPOutData & outPluginData, bool result, const QString & errorDesc)
{
	bool curResult = false;

	//qDebug() << "OnMakeMarkingFinished result:" << result << " marking size:" << outPluginData.Marking.size();

	pluginProgressCount--;
	if ( pluginProgressCount <= 0 )
		ui.progressBar_pluginDebug->hide();

	QString str = result ? "" : " error: " + errorDesc;

	SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender());
	OnProgeressMessage(id, 100, QString::fromUtf8("Получение разметки плагина: %1 завершено %2").arg(plugin->GetPluginName()).arg(str));
	PhrasesList * phrasesList = NULL;


	if ( result && tempPluginDataInt.find(id) != tempPluginDataInt.end() )
	{
		FileView * curView = tempPluginDataInt.take(id).View;
		PhraseController * controller = curView->GetPhraseController();

		phrasesList = controller->AddList(); 
		QString str = plugin->GetPluginName() + " marking";
		phrasesList->SetName(str);
		phrasesList->SetFileName(str);
		phrasesList->GetData().SetIntersected(false);

		QList<Phrase *> data;

		long max = 0;
		for (QList<SUPPhraseData>::const_iterator iter = outPluginData.Marking.constBegin(); 
			iter != outPluginData.Marking.constEnd(); iter++ )
		{
			Phrase * phrase = new Phrase(phrasesList->GetData(), iter->Start, iter->End, iter->Name);
			data.append(phrase);
			if ( iter->End > max )
				max = iter->End;
		}

		if ( data.size() && 
			( (max <= curView->GetWaveWidget()->GetWaveData()->GetCount()) || 
				(curMode == ConsoleCutVad) ||
				(QMessageBox::question(this, QString::fromUtf8("Внимание!"), 
				QString::fromUtf8("Границы разметки превышают (%1 > %2)границы файла, разметка будет обрезана по длинне файла, загрузить разметку?").arg(max).arg(curView->GetWaveWidget()->GetWaveData()->GetCount()),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) ) )
		{
			phrasesList->GetData().SetPhrases(data);
			curResult = true;
			phrasesList->GetData().SetCurPhrase(phrasesList->GetData().GetPhrases()[0]);
			phrasesList->GetData().setFocus();
		}
		else 
		{
			if ( !data.size() )
			{
				if ( curMode != ConsoleCutVad )
					QMessageBox::information(this, QString::fromUtf8("Внимание"),
							QString::fromUtf8("Неудалось создать разметку!!!"));
				else 
					std::cout << "Cannot create marking!!!" << std::endl;
			}

			controller->RemoveList(phrasesList);
			foreach(Phrase * phrase, data)
				delete phrase;
		}
	}
	else
	{
		if ( curMode != ConsoleCutVad )
			QMessageBox::information(this, QString::fromUtf8("Внимание"),
				QString::fromUtf8("Неудалось создать разметку!!!"));
		else
		{
			std::cout << "Cannot create marking!!!" << std::endl;
			std::cout << "No have speech data!!!" << std::endl;
		}
	}

	//Обрезаем шум, сохраняем в файл и выходим из программы.
	if ( curMode == ConsoleCutVad )
	{

		if ( curResult && phrasesList )
		{
			QList< Phrase *> speech_list = phrasesList->GetData().GetPhrases();

			if ( !speech_list.size() )
			{
				std::cout << "No speech data!!!" << std::endl;
				QCoreApplication::exit(2);
				return;
			}


			long commonLen = 0;
			QString lastDictor = speech_list.first()->GetName();

			for ( QList< Phrase *>::iterator iter = speech_list.begin();
				iter != speech_list.end(); iter++ )
			{
				commonLen = commonLen + (*iter)->GetLen();

				//Проверка дикторов
				if ( useDictorCount && 
					 lastDictor != (*iter)->GetName() )
				{
					std::cout << "Different speakers!!!!" << std::endl;
					QCoreApplication::exit(4);
					return;
				}
			}

			float fLen = commonLen /8000.f;
			
			if ( fLen < minSpeechDur )
			{
				std::cout << "Not enough speech data: " << fLen << " < " << minSpeechDur << " (sec)" << std::endl;
				QCoreApplication::exit(3);
				return;
			}


			DataFrame * frame = ui.tabWidget->GetCurDataFrame();
			if ( !frame )
			{
				std::cout << "Unknown error" << std::endl;
				QCoreApplication::exit(1);
				return;
			}

			QByteArray sound;

			if ( !makeXml )
			{
				for ( QList< Phrase *>::iterator iter = speech_list.begin();
					iter != speech_list.end(); iter++ )
				{
					QByteArray speechData;
					frame->GetData(speechData, (*iter)->GetStart(), (*iter)->GetLen());
					sound += speechData;
				}
			}
			//Создадим отчёт
			else 
			{
				QDomDocument doc("Report");
				QDomElement marking(doc.createElement("marking"));
				doc.appendChild(marking);
				
				foreach(const Phrase * ph, speech_list)
				{
					QDomElement phrase = doc.createElement("phrase");
					marking.appendChild(phrase);
					phrase.setAttribute("name", "speech");
					phrase.setAttribute("start", ph->GetStart());
					phrase.setAttribute("end", ph->GetEnd());
				}

				QFileInfo fInfo(outFileName);

				QFile data(fInfo.path() + "\\" + fInfo.completeBaseName() + ".xml");
				if (data.open(QFile::WriteOnly)) 
				{
					QTextStream out(&data);
					out.setCodec("UTF-8");
					out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
					out << doc.documentElement();
				}
				sound = frame->GetData();
			}

			std::string str = ui.tabWidget->GetCurFileView() ? ui.tabWidget->GetCurFileView()->GetFileName().toStdString() : "xxx";

			if ( SaveFile(outFileName, sound, frame->GetSampleRate(), frame->GetBitPersample()) )
			{
				std::cout << "File: " << str << " processed ok, result in: " << outFileName.toStdString() << std::endl;
				QCoreApplication::exit(0);
			}
			else
			{
				std::cout << "Cannot save in: " << outFileName.toStdString() << " processed file: " << str << std::endl;
				QCoreApplication::exit(1);
			}
		}
		else 
		{
			std::string str = ui.tabWidget->GetCurFileView() ? ui.tabWidget->GetCurFileView()->GetFileName().toStdString() : "xxx";
			std::cout << "Cannot process file: " << str << std::endl;
			QCoreApplication::exit(1);
		}
	}

}

void MainWindow::OnBatchProcessingStarted(int id)
{
	if ( settingsDialog->ShowPluginMassages() )
		on_action_showPluginFrame_triggered(true);
	ui.progressBar_pluginDebug->show();
	pluginProgressCount++;

	SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender());
	OnProgeressMessage(id, 0, QString::fromUtf8("Пакетная обработка плагина: %1 запущена.").arg(plugin->GetPluginName()));

}


void MainWindow::on_action_selectNextMarking_triggered(bool checked)
{
	if ( PhraseController * controller = ui.tabWidget->GetCurPhraseController() )
	{
		PhrasesList * list = controller->GetCurList();
		if ( !list && controller->GetLists().size() )
			SelestFirstPhrase(controller, controller->GetLists()[0]);
		else if ( list )
		{
			int index = (controller->GetLists().indexOf(list) + 1) % controller->GetLists().size();
			SelestFirstPhrase(controller, controller->GetLists()[index]);
		}
	}
}

void MainWindow::SelestFirstPhrase(PhraseController * controller, PhrasesList * list)
{
	if ( controller &&  list)
	{
		controller->SetCurList(list);
		PhrasesListData & data = controller->GetCurList()->GetData();
		if ( data.GetPhrases().size() )
			data.SetCurPhrase(data.GetPhrases()[0]);
	}
}

void MainWindow::OnBatchProcessingFinished(int id, bool result, const QString & errorDesc)
{
	pluginProgressCount--;
	if ( pluginProgressCount <= 0 )
		ui.progressBar_pluginDebug->hide();
	
	SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(sender());
	OnProgeressMessage(id, 100, QString::fromUtf8("Пакетная обработка плагина: %1 завершилась, %2").arg(plugin->GetPluginName()).arg(errorDesc));
}

void MainWindow::AddPluginMessage(const QString & message)
{
	ui.plainTextEdit_pluginDebug->appendPlainText(message);
	logStream << message << "\n";
	logStream.flush();
	messageCounter++;
}

void MainWindow::OnProgeressMessage(int ID, qreal progress, const QString & message)
{
	AddPluginMessage(QString("%1) %2: %3").arg(messageCounter).arg(QTime::currentTime().toString("hh:mm:ss.zzz")).arg(message));
}

void MainWindow::OnFileProcessingResult(int id, const SUPFileResult & result)
{
	QString message = QString("%1) %2: %3").
			arg(messageCounter).
			arg(QTime::currentTime().toString("hh:mm:ss.zzz")).
			arg(result.Result ? QString::fromUtf8("Файл \"%1\" обработан").arg(result.FileName) : 
				QString::fromUtf8("Файл \"%1\" не обработан, ошибка: %2").arg(result.FileName).
					arg(result.ErrorDesc));
	
	AddPluginMessage(message);
}

void MainWindow::OnStartFileProcessing(int, const QString & fileName)
{
	QString message = QString::fromUtf8("%1) %2: Начало обработки файла: %3").
			arg(messageCounter).
			arg(QTime::currentTime().toString("hh:mm:ss.zzz")).
			arg(fileName);
	
	AddPluginMessage(message);
}


void MainWindow::on_action_exit_triggered(bool checked)
{
	close();
}

bool TryOpenFileAs8bit16k(const QString & fileName,
						  QByteArray & sound,
						  SoundRecorder::WaveFormat & soundFormat)
{
	QString out_file_name("16bit8k.wav");
	QString cmd(QString("sox \"%1\" -r 8k -b 16 -c 1 \"%2\"").arg(fileName).arg(out_file_name));

	int res = system(cmd.toLocal8Bit().data());
	if ( res != 0 )
	{
		qWarning() << "Sox error:" << res;
		return false;
	}
	QFile file(out_file_name);

	if ( !SoundRecorder::ReadWaveFile(&file, soundFormat, sound) ||
		soundFormat.bitsPerSample != 16 ||
		soundFormat.sampleRate != 8000 ||
		soundFormat.channels != 1 )
	{
		qWarning() << "Cannot read file:" << out_file_name;
		sound.clear();
		QFile::remove(out_file_name);
		return false;
	}
	QFile::remove(out_file_name);
	return true;
}

bool MainWindow::OpenFile(const QString & fileName)
{
	if ( !fileName.isEmpty() && 
		QFileInfo(fileName).suffix().toLower() == "wav" )
	{
		QByteArray sound;
		QFile file(fileName);

		if ( !SoundRecorder::ReadWaveFile(&file, soundFormat, sound))
		{
			QMessageBox::information(this, QString::fromUtf8("Внимание"),
									QString::fromUtf8("Неудалось загрузить файл \"%1\"").arg(fileName));
			sound.clear();
			return false;
		}

		if ( !(soundFormat.bitsPerSample == 16 &&
			 (soundFormat.sampleRate == 8000 || soundFormat.sampleRate == 16000) &&
			 soundFormat.channels == 1) )
		{
			sound.clear();
			if( !TryOpenFileAs8bit16k(fileName, sound, soundFormat) )
				return false;
		}
		settings.setValue("lastFileName", fileName);

		FileView * view = ui.tabWidget->AddFileView(fileName);
		view->GetPhraseController()->SavePhraseTextWE(settingsDialog->IsSavePhraseTextWE());
		view->GetPhraseController()->SetMarkingFont(settingsDialog->GetMarkingFont());

		view->GetWaveWidget()->GetWaveData()->SetData(sound, soundFormat.bitsPerSample, soundFormat.sampleRate);
		view->GetWaveWidget()->SetWindow(view->GetWaveWidget()->GetMaxWindow());
		CreateAudioOutput(soundFormat);
 		Stop();
		controlsFrame->SetEnabledPlayback(true);
		controlsFrame->SetSampleRate(soundFormat.sampleRate);

		//Откроем одноимённёю разметку.
		if ( settingsDialog->OpenMarkingSameWave() )
		{
			QFileInfo fi(fileName);
			QStringList lab_list = QDir(fi.path()).entryList(QStringList(fi.completeBaseName() + ".lab*"),
										QDir::Files);

			for(QStringList::iterator iter = lab_list.begin(), end = lab_list.end();
				iter != end; iter++ )
			{
				LoadMarking(fi.path() + "/" + *iter);

				QString file_name = fi.path() + "/" + fi.completeBaseName() + ".json";
				
				if ( signal_desc_frame->LoadFromFile(file_name) )
				{
					view->SetSignalDesc(signal_desc_frame->GetDesc());
					view->SetSignalDescFilePath(file_name);
				}
			}
		}
		on_action_showTime_triggered(true);
		return true;
	}
	return false;
}

bool MainWindow::OpenFilesAsOne(const QStringList & files)
{
	QByteArray sound;
	QStringList tempNames;

	for ( QStringList::const_iterator iter =  files.constBegin(); 
		iter != files.constEnd(); iter++ )
	{
		if ( !iter->isEmpty() && QFileInfo(*iter).suffix().toLower() == "wav" )
		{
			QFile file(*iter);
			QByteArray buffer;
			if ( !SoundRecorder::ReadWaveFile(&file, soundFormat, buffer))
			{
				std::cout << "Cannot load file:" << iter->toAscii().data() << std::endl;
				continue;
			}

			if ( soundFormat.bitsPerSample != 16 || soundFormat.sampleRate != 8000 || soundFormat.channels != 1 )
			{
				std::cout << "Cannot load file:" << iter->toAscii().data() << ", wrong file format (only:1 channel, 16 bit per sample, 8000 hz)" << std::endl;
				continue;
			}
			sound += buffer;
			tempNames << QFileInfo(*iter).fileName();
		}
	}

	if ( sound.size() )
	{
		QString fileName("composite(");
		QStringList::iterator iter = tempNames.begin(); 
		QStringList::iterator next = iter;

		for (;iter != tempNames.end(); iter++ )
		{
			next++;
			fileName += *iter + (next == tempNames.end() ? ")" : " ");
		}

		FileView * view = ui.tabWidget->AddFileView(fileName);
		view->GetPhraseController()->SavePhraseTextWE(settingsDialog->IsSavePhraseTextWE());
		view->GetPhraseController()->SetMarkingFont(settingsDialog->GetMarkingFont());

		view->GetWaveWidget()->GetWaveData()->SetData(sound, 16, 8000);
		view->GetWaveWidget()->SetWindow(view->GetWaveWidget()->GetMaxWindow());
		Stop();
		CreateAudioOutput(soundFormat);
		controlsFrame->SetEnabledPlayback(true);
		controlsFrame->SetSampleRate(8000);
		return true;
	}
	return false;
}

bool MainWindow::SaveFile(const QString & fileName)
{
	if ( !fileName.isEmpty() )
	{
		if ( DataFrame * data =  ui.tabWidget->GetCurDataFrame() )
		{
			QByteArray sound;
			QFile file(fileName);
			return SoundRecorder::WriteWaveFile(&file, 1, data->GetSampleRate(), data->GetBitPersample(), data->GetData());
		}
	}
	return false;
}

bool MainWindow::SaveFile(const QString & fileName, 
						  const QByteArray & sound, 
						  int sampleRate,
						  int bitPersample)
{
	if ( !fileName.isEmpty() )
	{
		QFile file(fileName);
		return SoundRecorder::WriteWaveFile(&file, 1, sampleRate, bitPersample, sound);
	}
	return false;
}


void MainWindow::on_action_openFile_triggered(bool checked)
{
	QStringList files = QFileDialog::getOpenFileNames(this, 
													 QString::fromUtf8("Открыть звуковой файл"),
													 settings.value("lastFileName","").toString(),
													QString::fromUtf8("Звук (*.wav *.WAV)"));
	foreach(QString fileName, files)
		OpenFile(fileName);
}

void MainWindow::on_action_showAll_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget())
		widget->SetWindow(widget->GetMaxWindow());
}

void MainWindow::OnSelectedSamplesChanged(long startFrame, long endFrame)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		UpdatePlayPos(widget->GetCurrentPlayPos());
	}
	controlsFrame->OnSelectedChanged(startFrame, endFrame);
}


void MainWindow::OnNotify()
{
	if ( DataFrame * frame = ui.tabWidget->GetCurDataFrame() )
	{
		long samples = audioOutput->processedUSecs() / (1./frame->GetSampleRate() * 1000000) + offset;
		ui.tabWidget->GetCurWaveWidget()->SetCurrentPlayPos(samples);
		UpdatePlayPos(samples);
	}
}

void MainWindow::UpdatePlayPos(long samples)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		controlsFrame->OnPlayPosChanged(samples - widget->GetSelectedSamples().first);
	else 
		controlsFrame->OnPlayPosChanged(0);
}

void MainWindow::OnStateChanged(QAudio::State state)
{
	if (state == QAudio::StoppedState)
		Stop();
}


void MainWindow::onPauseClicked()
{
	if ( audioOutput )
	{
		if ( audioOutput->state() == QAudio::SuspendedState )
		{
			if ( DataFrame * frame = ui.tabWidget->GetCurDataFrame() )
			{
				if ( settingsDialog->UseOffsetAfterPause() ) 
				{
					quint32 offset = (settingsDialog->GetOffsetAfterPause()/1000.) * frame->GetSampleRate() * frame->GetBitPersample()/8;
					audioOutput->SetCurPos(audioOutput->GetCurPos() + offset);
				}
			}
			audioOutput->resume();
		}
		else 
			audioOutput->suspend();
	}
}


bool MainWindow::Play()
{
	//std::cout << "Play" << std::endl;
	if ( audioOutput )
	{
		if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		{
			audioOutput->stop();

			QPair<long, long> samples = widget->GetSelectedSamples();
			samples.second = samples.first == samples.second ? widget->GetWaveData()->GetCount() : samples.second;

			QByteArray data;
			if ( widget->GetWaveData()->GetData(data, samples.first, samples.second - samples.first) )
			{
				widget->SetVisibleCurrentPlayPos(true);
				offset = samples.first;
				widget->SetCurrentPlayPos(samples.first);
				audioOutput->SetFrequency(widget->GetWaveData()->GetSampleRate() * settingsDialog->GetPlaySpeed());
				audioOutput->start(data);
				widget->EnablePlayPosTracking(settingsDialog->IsPlayPosTracking());

				controlsFrame->OnPlayChanged(true);
				return true;
			}
		}
	}
	return false;
}

bool MainWindow::Stop()
{
	//std::cout << "Stop" << std::endl;
	if ( audioOutput )
	{
		audioOutput->stop();
		if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
			widget->EnablePlayPosTracking(false);

		controlsFrame->OnPlayChanged(false);
		return true;
	}
	return false;
}

bool MainWindow::IsPlayed()
{
	if ( audioOutput )
		return audioOutput->state() == QAudio::ActiveState || audioOutput->state() == QAudio::SuspendedState;
	return false;
}


void MainWindow::onStartSelectedChanged(long value)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		widget->SelectSamples(value, widget->GetSelectedSamples().second, false);
	}
}

void MainWindow::onEndSelectedChanged(long value)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		widget->SelectSamples(widget->GetSelectedSamples().first, value, false);
	}
}


void MainWindow::OnPosChanged(long sample, double amplitude)
{
	this->statusBar()->showMessage(QString("S: %1 A: %2").arg(sample).arg(amplitude));
}

void MainWindow::UpdatePhraseData(bool checkWindow )
{
	if ( Phrase * phrase = ui.tabWidget->GetCurPhrase() )
	{
		//Сохраняем разметку автоматически.
		if ( settingsDialog->IsAutoSaveMarking() )
			on_action_saveMarking_triggered(true);

		controlsFrame->SetEnabledPhrase(true);
		ui.action_delPhrase->setEnabled(true);
		ui.action_cutPhrase->setEnabled(true);

		controlsFrame->OnPhraseChanged(phrase->GetStart(), phrase->GetEnd(), phrase->GetName());
		ui.tabWidget->GetCurWaveWidget()->SelectSamples(phrase->GetStart(), phrase->GetEnd(),checkWindow);
	}
	else 
	{
		controlsFrame->SetEnabledPhrase(false);
		ui.action_delPhrase->setEnabled(false);
		ui.action_cutPhrase->setEnabled(false);	
		controlsFrame->OnPhraseChanged(0,0,"");
	}
}
void MainWindow::OnCurrentPhraseChanged()
{
	UpdatePhraseData(settingsDialog->IsAutoScalePlot());
	if ( settingsDialog->IsPlayAfterChangePhrase() )
		Play();
}

void MainWindow::on_action_PrepareForTrans_triggered(bool)
{
	on_action_addMarking_triggered(true);
	on_action_selectAll_triggered(true);
	on_action_addPhrase_triggered(true);
	on_action_editPhraseText_triggered(true);
}


void MainWindow::OnPhraseDataChanged()
{
	UpdatePhraseData(false);
}

void MainWindow::on_action_searchPhrase_triggered(bool checked)
{
	if ( Phrase * phrase = ui.tabWidget->GetCurPhrase() )
		searchDialog->SetSearchName(phrase->GetName());
	searchDialog->show();
}

void MainWindow::OnFindNext()
{
	if( Phrase * phrase = ui.tabWidget->GetCurPhraseController()->SearchPhrase(searchDialog->GetSearchName(),
				searchDialog->IsCaseSensitive(),
				searchDialog->IsWholeWord(),
				(PhraseController::SearchType)searchDialog->GetSearchType()) )
	{
		ui.tabWidget->GetCurPhraseController()->SetCurPhrase(phrase);
	}
}


void MainWindow::on_action_closeFile_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
		if ( QMessageBox::question(this, QString::fromUtf8("Внимание!"), 
						QString::fromUtf8("Фонограмма и разметки будут закрыты. Продолжить?"),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes )
	{
		ui.tabWidget->DelFileView(view);

		//Удалим указатель на вид.
		QList<int> temp;
		for (QMap<int, TempPluginData>::iterator iter = tempPluginDataInt.begin(); 
			iter != tempPluginDataInt.end(); iter++ )
			if ( iter->View == view )
				temp.append(iter.key());
		foreach(int k, temp)
			tempPluginDataInt.remove(k);
	}
}

void MainWindow::on_action_closeAllFiles_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
		if ( QMessageBox::question(this, QString::fromUtf8("Внимание!"), 
			QString::fromUtf8("Все файлы(%1) будут удалены. Продолжить?").arg(ui.tabWidget->count()),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes )
	{
		ui.tabWidget->ClearAll();
	}
}


void MainWindow::on_action_setupGradient_triggered(bool checked)
{
	GradientDialog dialog;
	dialog.exec();
}

void MainWindow::OnChangeType()
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		if ( ui.radioButton_wave->isChecked() )
			widget->ShowWave(true);
		else 
			widget->ShowWave(false);
	}
}

void MainWindow::OnCurViewChanged()
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		controlsFrame->setEnabled(true);
		controlsFrame->SetSampleRate(view->GetWaveWidget()->GetWaveData()->GetSampleRate());

		signal_desc_frame->setEnabled(true);
		ui.toolBar_signal_desc->setEnabled(true);
		signal_desc_frame->SetDesc(view->GetSignalDesc().second);

		Stop();

		QPair<long, long> samples = view->GetWaveWidget()->GetSelectedSamples();
		OnSelectedSamplesChanged(samples.first, samples.second);
		ui.frame_type->setEnabled(true);

		if ( view->GetWaveWidget()->IsWave() )
			ui.radioButton_wave->setChecked(true);
		else 
			ui.radioButton_spector->setChecked(true);
		
		ui.frame_buttons->setEnabled(true);

		if ( view->GetWaveWidget()->IsDisplaySamples() )
		{
			ui.action_showSample->blockSignals(true);
			ui.action_showTime->blockSignals(true);

			ui.action_showSample->setChecked(true);
			ui.action_showTime->setChecked(false);

			ui.action_showSample->blockSignals(false);
			ui.action_showTime->blockSignals(false);
		}
		else 
		{
			ui.action_showSample->blockSignals(true);
			ui.action_showTime->blockSignals(true);

			ui.action_showSample->setChecked(false);
			ui.action_showTime->setChecked(true);

			ui.action_showSample->blockSignals(false);
			ui.action_showTime->blockSignals(false);
		}
		setWindowTitle(view->GetShortFileName());
	}
	else
	{
		signal_desc_frame->setEnabled(false);
		ui.toolBar_signal_desc->setEnabled(false);
		signal_desc_frame->SetDesc("");

		OnSelectedSamplesChanged(0, 0);
		ui.frame_type->setEnabled(false);
		controlsFrame->setEnabled(false);
		ui.frame_buttons->setEnabled(false);
		setWindowTitle("");
	}
	ui.action_closeFile->setVisible(ui.tabWidget->count());
	ui.action_closeAllFiles->setVisible(ui.tabWidget->count() > 1);

	OnCurrentPhraseChanged();
	OnCurrentListChanged();

}

void MainWindow::on_action_showPluginFrame_triggered(bool checked)
{
	ui.frame_pluginDebug->show();
}

void MainWindow::on_action_hidePluginFrame_triggered(bool checked)
{
	ui.frame_pluginDebug->hide();
}

void MainWindow::on_action_clearPluginMessages_triggered(bool checked)
{
	ui.plainTextEdit_pluginDebug->clear();
	messageCounter = 1;
}

void MainWindow::on_action_copyPluginMessages_triggered(bool checked)
{
	ui.plainTextEdit_pluginDebug->copy();
}

void MainWindow::on_action_openSettings_triggered(bool checked)
{
	if ( settingsDialog->exec() == QDialog::Accepted )
	{
		for ( int i = 0; i < ui.tabWidget->count(); i++ )
		{
			FileView  * view = (FileView *)ui.tabWidget->widget(i);
			view->GetPhraseController()->SavePhraseTextWE(settingsDialog->IsSavePhraseTextWE());
			view->GetPhraseController()->SetMarkingFont(settingsDialog->GetMarkingFont());
		}
	}
}

void MainWindow::on_action_editPhraseText_triggered(bool checked)
{
	if ( PhrasesListData * data = ui.tabWidget->GetCurPhrasesListData() )
	{
		if ( settingsDialog->IsEditPhraseNameInMarking() )
			data->EditPhraseName(data->GetCurPhrase());
		else
		{
			controlsFrame->SetPhraseNameFocus();
		}
	}
}

void MainWindow::onPlaybackChanged(bool play)
{
	if ( play )
		Play();
	else 
	{
		if ( Stop() && settingsDialog->IsAutoMoveMarker() )
		{
			if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
					widget->SelectSamples(widget->GetCurrentPlayPos(),widget->GetCurrentPlayPos(), false);
		}	
	}
}

void MainWindow::on_action_moveLeft_leftMarkerBorder_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		widget->MoveLeft_LeftMarkerBorder();
}


void MainWindow::on_action_zoom_in_vertical_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		widget->ZoomInVertical();
	}
}

void MainWindow::on_action_zoom_out_vertical_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		widget->ZoomOutVertical();
	}
}

void MainWindow::on_action_zoom_in_horizontal_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		widget->ZoomInHorizontal();
	}
}

void MainWindow::on_action_zoom_out_horizontal_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		widget->ZoomOutHorizontal();
	}
}


void MainWindow::on_action_cutFile_triggered(bool checked)
{
	DataFrame * frame = ui.tabWidget->GetCurDataFrame();
	PhrasesListData * data = ui.tabWidget->GetCurPhrasesListData();
	FileView * view = ui.tabWidget->GetCurFileView();

	if ( frame && data )
	{
		QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("Установить для результата"),
												settings.value("lastCutFileDir", "").toString(),
												QFileDialog::ShowDirsOnly
												| QFileDialog::DontResolveSymlinks);

		if ( !dir.isEmpty() )
		{
			settings.setValue("lastCutFileDir",dir);
			const QList<Phrase * > & list = data->GetPhrases();

			int counter = 1;
			char ssss[100];
			for ( QList<Phrase * >::const_iterator iter = list.constBegin(); 
				iter != list.constEnd(); iter++ )
			{
				QByteArray sound;
				frame->GetData(sound, (*iter)->GetStart(), (*iter)->GetLen());
				sprintf(ssss, "\\%s_%02i", QFileInfo(view->GetShortFileName()).completeBaseName().toLocal8Bit().data(), counter);
				QFile file(dir + ssss+".wav");
				SoundRecorder::WriteWaveFile(&file, 1, frame->GetSampleRate(), frame->GetBitPersample(), sound);

				QFile labFile(dir + ssss+".txt");
				labFile.open(QIODevice::WriteOnly | QIODevice::Text);
				labFile.write((*iter)->GetName().toLocal8Bit());
				counter++;
			}
		}
	}
}


void MainWindow::on_action_moveRight_leftMarkerBorder_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		widget->MoveRight_LeftMarkerBorder();
}

void MainWindow::on_action_moveLeft_rightMarkerBorder_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		widget->MoveLeft_RightMarkerBorder();
}

void MainWindow::on_action_moveRight_rightMarkerBorder_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		widget->MoveRight_RightMarkerBorder();
}

void MainWindow::on_action_saveFileAs_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		QString fileName = QFileDialog::getSaveFileName(this, QString::fromUtf8("Сохранить файл"),
														view->GetFileName(),
														QString::fromUtf8("Звук (*.wav *.WAV)"));
		view->SetFileName(fileName);
		SaveFile(fileName);
	}
}

void MainWindow::on_action_save_file_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
		SaveFile(view->GetFileName());
}



void MainWindow::on_action_removePhrasesByName_triggered(bool checked)
{
	if ( PhrasesListData * data = ui.tabWidget->GetCurPhrasesListData() )
	{
		RemovePhrasesByNameDialog dialog(settings);
		if ( dialog.exec() == QDialog::Accepted )
		{
			QList< Phrase * > list = data->GetPhrasesByName(dialog.GetPhrasesName(), dialog.IsCaseSensitive());
			PhraseDelDialog::ResultCode code = dialog.IsRemovePhrasesAndSignal() ? PhraseDelDialog::RemoveWithFrame : PhraseDelDialog::NoMove;

			for ( QList< Phrase *>::iterator iter = list.begin();
				iter != list.end(); iter++ )
				RemovePhraseByType(*data, (*iter), code);
	
			CheckPhrasesDuraton();
		}
	}
}

void MainWindow::on_action_removeUnmarks_triggered(bool checked)
{
	if ( QMessageBox::question(this, QString::fromUtf8("Внимание!"), 
					QString::fromUtf8("Внимание не размеченные участки будут удалены. Продолжить?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes )
					return;


	if ( PhrasesListData * data = ui.tabWidget->GetCurPhrasesListData() )
	{
		long begin = 0;
		long end = 0;
		const QList< Phrase * > & list = data->GetPhrases();
		PhraseController * phraseController = ui.tabWidget->GetCurPhraseController();
		DataFrame * dataFrame = ui.tabWidget->GetCurDataFrame();
		
		for ( int i = 0; i < list.size(); i++ )
		{
			end = list[i]->GetStart();
			phraseController->RemoveFragment(begin, end);
			dataFrame->RemoveData(begin, end - begin);
			begin = list[i]->GetEnd();
		}

		//удалим конец.
		end = dataFrame->GetCount();
		phraseController->RemoveFragment(begin, end);
		dataFrame->RemoveData(begin, end - begin);

		CheckPhrasesDuraton();
	}
}



void MainWindow::OnDropWavFile(const QString & fileName)
{
	OpenFile(fileName);
}

void MainWindow::OnDropMarkingFile(const QString & fileName)
{
	LoadMarking(fileName);
}

void MainWindow::on_action_nextPage_triggered(bool checked)
{
	ui.tabWidget->setCurrentIndex((ui.tabWidget->currentIndex() + 1) % ui.tabWidget->count());
}

void MainWindow::on_action_previousPage_triggered(bool checked)
{
	int index = ui.tabWidget->currentIndex() - 1;
	ui.tabWidget->setCurrentIndex(index < 0 ? ui.tabWidget->count() - 1 : index);
}

void MainWindow::OnCurrentListChanged()
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
	{
		controlsFrame->SetEnabledMarking(list->IsEnable());
		ui.action_addPhrase->setEnabled(true);
		ui.action_setPhrasesIntersection->setEnabled(true);
		ui.action_saveMarking->setEnabled(true);
		ui.action_saveMarkingAs->setEnabled(true);
		ui.action_searchPhrase->setEnabled(true);
		ui.action_removePhrasesByName->setEnabled(true);
		ui.action_delMarking->setEnabled(true);

		controlsFrame->OnMarkingChanged(list->GetName(), 
										list->GetData().IsIntersected(), 
										list->GetData().GetColor(),
										list->GetFileName(),
										list->GetData().GetString());
		list->GetData().setFocus();
	}
	else 
	{
		controlsFrame->SetEnabledMarking(false);
		controlsFrame->SetEnabledPhrase(false);

		ui.action_addPhrase->setEnabled(false);
		ui.action_setPhrasesIntersection->setEnabled(false);

		controlsFrame->OnMarkingChanged("", 
										false, 
										QColor(),
										"",
										"");

		ui.action_delPhrase->setEnabled(false);
		ui.action_cutPhrase->setEnabled(false);
		ui.action_saveMarking->setEnabled(false);
		ui.action_saveMarkingAs->setEnabled(false);
		ui.action_searchPhrase->setEnabled(false);
		ui.action_removePhrasesByName->setEnabled(false);
		ui.action_delMarking->setEnabled(false);
	}
}




void MainWindow::onStartPhraseChanged(long value)
{
	if ( Phrase * phrase = ui.tabWidget->GetCurPhrase() )
		phrase->SetStart(value);
}

void MainWindow::onEndPhraseChanged(long value)
{
	if ( Phrase * phrase = ui.tabWidget->GetCurPhrase() )
		phrase->SetEnd(value);
}

void MainWindow::onPhraseNameChanged(const QString & name)
{
	if ( Phrase * phrase = ui.tabWidget->GetCurPhrase() )
		phrase->SetName(name);
}


void MainWindow::on_action_addPhrase_triggered(bool checked)
{
	if ( PhrasesListData * data = ui.tabWidget->GetCurPhrasesListData() )
	{
		QPair<long,long> s = ui.tabWidget->GetCurWaveWidget()->GetSelectedSamples();
		if ( s.first != s.second )
		{
			Phrase * phrase = new Phrase(*data, s.first, s.second, "New");
			data->InsertPhrase(phrase, settingsDialog->IsCopyPhraseTextAC());
		}
		else 
		{
			data->CutPhrases(s.first,settingsDialog->IsCopyPhraseTextAC());
		}
		data->setFocus();
	}
}

void MainWindow::on_action_delPhrase_triggered(bool checked)
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
	{
		if ( Phrase * phrase = list->GetData().GetCurPhrase() )
		{
			PhraseDelDialog dialog;
			RemovePhraseByType(list->GetData(), phrase, (PhraseDelDialog::ResultCode)dialog.exec());
			CheckPhrasesDuraton();
		}
	}
}

bool MainWindow::RemovePhraseByType(PhrasesListData & list, Phrase * phrase, PhraseDelDialog::ResultCode code)
{
	bool res = false;

	if ( phrase )
	{
		if ( code == PhraseDelDialog::MoveLeft )
			res =list.RemovePhrase(phrase, PhrasesListData::MoveLeftBorder);
		else if ( code == PhraseDelDialog::MoveRight )
			res = list.RemovePhrase(phrase, PhrasesListData::MoveRightBorder);
		else if ( code == PhraseDelDialog::NoMove )
			res = list.RemovePhrase(phrase, PhrasesListData::NoMoveBorder);
		else if ( code == PhraseDelDialog::RemoveWithFrame )
		{
			long start  = phrase->GetStart();
			long end = phrase->GetEnd();
			list.RemoveFragment(start, end);
			ui.tabWidget->GetCurDataFrame()->RemoveData(start, end - start);
			ui.tabWidget->GetCurWaveWidget()->SelectSamples(start, start, false);
			res = true;
		}
	}
	return res;
}

void MainWindow::CheckPhrasesDuraton()
{
	if ( settingsDialog->IsCheckPhrasesDuratio() && curMode != ConsoleCutVad )
	{
		DataFrame * frame = ui.tabWidget->GetCurDataFrame();
		PhrasesListData * data = ui.tabWidget->GetCurPhrasesListData();

		if ( frame && data )
		{
			QList< Phrase * > list = data->GetPhrasesByName(settingsDialog->GetPhrasesName(), true);
			long len = 0;
			
			for ( QList< Phrase * >::iterator iter = list.begin(); 
				iter != list.end(); iter++ )
				len = len + (*iter)->GetLen();

			len = (len/(qreal)frame->GetSampleRate()) * 1000.;

			if ( len < settingsDialog->GetPhraseDurationThreshold() )
			{
				QMessageBox::warning(this, QString::fromUtf8("Внимание"), 
					QString::fromUtf8("Длительность фраз \"%1\" в текущей разметки меньше установленного порога: %2 < %3 (мс)").
					arg(settingsDialog->GetPhrasesName()).arg(len).arg(settingsDialog->GetPhraseDurationThreshold()));
			}
		}
	}
}

void MainWindow::on_action_copyFrame_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		QPair<long, long> selected = widget->GetSelectedSamples();
		widget->GetWaveData()->GetData(copyData, selected.first, selected.second - selected.first + 1);
	}
}

void MainWindow::on_action_delFrame_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		Stop();

		if ( QMessageBox::question(this, QString::fromUtf8("Внимание!"), 
						QString::fromUtf8("Выделенный фрагмент фонограммы и фразы будут удалены. Продолжить?"),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes )
		{
			QPair<long, long> selected = widget->GetSelectedSamples();
			ui.tabWidget->GetCurPhraseController()->RemoveFragment(selected.first, selected.second);
			widget->GetWaveData()->RemoveData(selected.first, selected.second - selected.first);
			widget->SelectSamples(selected.first,selected.first, false);
			CheckPhrasesDuraton();
		}
	}
}

void MainWindow::on_action_insertFrame_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		if ( copyData.size() )
		{
			Stop();
			QPair<long, long> selected = widget->GetSelectedSamples();
			widget->GetWaveData()->RemoveData(selected.first, selected.second - selected.first);
			widget->GetWaveData()->InsertData(copyData, selected.first);
		
			int count = copyData.size()/(widget->GetWaveData()->GetBitPersample()/8);
			ui.tabWidget->GetCurPhraseController()->InsertFragment(selected.first, count);
			widget->SelectSamples(selected.first,count,false);
		}
	}
}

void MainWindow::on_action_selectAll_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
		widget->SelectSamples(0, widget->GetWaveData()->GetCount(), false);
}

void MainWindow::on_action_cutPhrase_triggered(bool checked)
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
	{
		list->GetData().CutPhrases(ui.tabWidget->GetCurWaveWidget()->GetSelectedSamples().first, settingsDialog->IsCopyPhraseTextAC());
	}
}

void MainWindow::on_action_showSample_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		controlsFrame->SetFormatType(checked ? ControlsFrame::Samples : ControlsFrame::Time);

		ui.action_showTime->setChecked(!checked);
		widget->DisplaySamples(checked);
		OnCurViewChanged();
		UpdatePlayPos(widget->GetCurrentPlayPos());
	}
}

void MainWindow::on_action_showTime_triggered(bool checked)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		controlsFrame->SetFormatType(checked ? ControlsFrame::Time : ControlsFrame::Samples);

		ui.action_showSample->setChecked(!checked);
		widget->DisplaySamples(!checked);
		OnCurViewChanged();
		UpdatePlayPos(widget->GetCurrentPlayPos());
	}
}

void MainWindow::onMarkingNameChanged(const QString & name)
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
		list->SetName(name);
}

void MainWindow::onMarkingIntersectionChanged(bool state)
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
		list->GetData().SetIntersected(state);
}

void MainWindow::onMarkingColorChanged(const QColor & color)
{
	if ( PhrasesListData * list = ui.tabWidget->GetCurPhrasesListData() )
		list->SetColor(color);
}

void MainWindow::on_action_addMarkingFormFile_triggered(bool checked)
{
	if ( DataFrame * frame = ui.tabWidget->GetCurDataFrame() )
	{
		if ( frame->GetCount() )
		{
			QString path = QFileInfo(QFileInfo(settings.value("lastMarkingFileName", "").toString()).path(),
				QString("%1.lab").arg(QFileInfo(ui.tabWidget->GetCurFileView()->GetFileName()).baseName())).filePath();
			
			QStringList files= QFileDialog::getOpenFileNames(this, QString::fromUtf8("Открыть файл разметки"),
															 path,
															 QString::fromUtf8("Разметка (*.LAB *.lab)"));
			foreach(QString fileName, files)
				LoadMarking(fileName);
		}
	}
}

void MainWindow::on_action_saveMarking_triggered(bool checked)
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
	{
		if ( !list->GetFileName().isEmpty() )
		{
			SaveMarking(list->GetFileName(), list->GetData().GetPhrases());
			qDebug() << "Save marking in file:" << list->GetFileName();
		}
		else
			on_action_saveMarkingAs_triggered(true);
	}
}


void MainWindow::on_action_saveMarkingAs_triggered(bool checked)
{
	if ( PhrasesList * list = ui.tabWidget->GetCurPhrasesList() )
	{
		QFileInfo info(!list->GetFileName().isEmpty() ? list->GetFileName() :
							ui.tabWidget->GetCurFileView()->GetFileName());

		QString fileName = QFileDialog::getSaveFileName(this, QString::fromUtf8("Сохранить разметку в файл"),
														info.absolutePath() + "/" + info.completeBaseName() + ".lab",
														 QString::fromUtf8("Разметка (*.LAB *.lab)"));
		if ( !fileName.isEmpty() )
		{
			SaveMarking(fileName, list->GetData().GetPhrases());
			list->SetFileName(fileName);
		}
	}
}


void MainWindow::OnPhraseDoubleClick()
{
	if ( Phrase * phrase = ui.tabWidget->GetCurPhrase() )
	{
		ui.tabWidget->GetCurWaveWidget()->SelectSamples(phrase->GetStart(), phrase->GetEnd(), false);
	}
}

void MainWindow::onMoveMarkingUp()
{
	if ( PhraseController * controller = ui.tabWidget->GetCurPhraseController() )
		controller->MoveUp();
}

void MainWindow::onMoveMarkingDown()
{
	if ( PhraseController * controller = ui.tabWidget->GetCurPhraseController() )
		controller->MoveDown();
}



void MainWindow::on_action_addMarking_triggered(bool checked)
{
	if ( PhraseController * controller = ui.tabWidget->GetCurPhraseController() )
		controller->AddList();
}

void MainWindow::on_action_delMarking_triggered(bool checked)
{
	if ( PhraseController * controller = ui.tabWidget->GetCurPhraseController() )
		controller->RemoveList(controller->GetCurList());
}

bool MainWindow::CreateAudioOutput(const SoundRecorder::WaveFormat & soundFormat)
{
	QAudioFormat format;
	format.setFrequency(soundFormat.sampleRate);
	format.setChannels(soundFormat.channels);
	format.setSampleSize(soundFormat.bitsPerSample);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
		//std::cout << "Default format not supported - trying to use nearest";
        format = info.nearestFormat(format);
    }

    delete audioOutput;
    audioOutput = 0;
    //audioOutput = new QAudioOutput(device, format, this);
	audioOutput = new AudioOutputDS(format, this);
	audioOutput->setNotifyInterval(10);

	connect(audioOutput, SIGNAL(notify()), SLOT(OnNotify()));
    connect(audioOutput, SIGNAL(stateChanged(QAudio::State)), SLOT(OnStateChanged(QAudio::State)));
	return true;
}

bool MainWindow::SetSettings(const QString & settings)
{
	json_spirit::mValue v;
	
	if ( json_spirit::read(settings.toUtf8().data(), v) )
	{
		try
		{
			json_spirit::mObject & obj = v.get_obj();
			json_spirit::mObject::iterator iter;
			
			//Тип фрейма 
			if ( (iter = obj.find("control_frame")) != obj.end() )
			{
				json_spirit::mObject & obj = iter->second.get_obj();
				
				if ( (iter = obj.find("type")) != obj.end() )
				{
					if ( iter->second.get_str() == "extcontrolsframe" &&
						dynamic_cast<ExtControlsFrame *>(controlsFrame) == NULL )
					{
						SetupControlsFrame(new ExtControlsFrame(this));
					}
					else if ( iter->second.get_str() == "simplecontrolsframe" && 
						dynamic_cast<ExtControlsFrame *>(controlsFrame) != NULL )
					{
						SetupControlsFrame(new SimpleControlsFrame(this));
					}
				}

				if ( (iter = obj.find("settings")) != obj.end() )
					controlsFrame->SetConfig(QString::fromStdString(json_spirit::write(iter->second.get_obj())));
			}

			if ( (iter = obj.find("objects")) != obj.end() )
			{
				json_spirit::mArray & arr = iter->second.get_array();
				
				for ( json_spirit::mArray::iterator iter = arr.begin();
					iter != arr.end(); iter++)
				{
					json_spirit::mObject & obj = iter->get_obj();
					
					if( QObject * cur_obj = findChild<QObject*>(obj["name"].get_str().c_str()) )
					{
						json_spirit::mObject & arr = obj["propertys"].get_obj();
						
						for ( json_spirit::mObject::iterator iter = arr.begin();
							iter != arr.end(); iter++ )
							cur_obj->setProperty(iter->first.c_str(), JsonValueToVariant(iter->second));
					}
				}
			}
			return true;
		}
		catch(std::exception & e)
		{
			qWarning() << "parsing error:" << e.what();
		}
	}
	else
		qWarning() << "cannot read settings";
	return false;
}

QVariant JsonValueToVariant(const json_spirit::mValue & v)
{
	switch( v.type() )
	{
		case json_spirit::bool_type:
			return v.get_bool();
		case json_spirit::str_type:
			return v.get_str().c_str();
		case json_spirit::int_type:
			return v.get_int();
		case json_spirit::real_type:
			return v.get_real();
		default:
			return QVariant();
	}
}

bool MainWindow::LoadMarking(const QString & fileName)
{
	PhraseController * controller = ui.tabWidget->GetCurPhraseController();
	WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget();

	if ( !controller || 
		!widget || 
		!widget->GetWaveData()->GetCount() )
		return false;

	QFileInfo info(fileName);
	QString suf(info.suffix().toLower());

	if ( QRegExp("lab\\d*").exactMatch(suf) )
	{
		QFile file(fileName);
		if ( file.open(QFile::ReadOnly) )
		{
			QString sdp;
			if (settingsDialog->OpenMarkingAsUTF8() )
				sdp = QString::fromUtf8(file.readAll());
			else
				sdp = QString::fromUtf8(file.readAll());

			QStringList lines = sdp.split(QRegExp("(\\n|\\r|\\r\\n)"));

			PhrasesList * phrasesList = controller->AddList(); 
			phrasesList->SetName(fileName);
			phrasesList->SetFileName(fileName);

			phrasesList->GetData().SetIntersected(false);

			QRegExp line("(\\d+)\\s+(\\d+) (.+)");
			QList<Phrase *> data;

			long max = 0;
			for (QStringList::iterator iter = lines.begin(); 
				iter != lines.end(); iter++ )
			{
				if ( line.indexIn(*iter) > -1 )
				{
					long start = widget->ToSample( ((qreal)line.cap(1).toULongLong()) * 0.0000001 );
					long stop = widget->ToSample( ((qreal)line.cap(2).toULongLong()) * 0.0000001 );
					QString name = line.cap(3);
					if ( stop < start )
						QMessageBox::information(this, QString::fromUtf8("Внимание!!"), 
							QString::fromUtf8("Значение конца метки \"%1\" в файле \"%2\" меньше чем начало: %3 < %4 (отсчёты). ").
							arg(name).arg(fileName).arg(stop).arg(start));

					Phrase * phrase = new Phrase(phrasesList->GetData(), start, stop, name);
					data.append(phrase);

					if ( stop > max )
						max = stop;
				}
			}
			if ( data.size() && 
				(max <= widget->GetWaveData()->GetCount() ||
					QMessageBox::question(this, QString::fromUtf8("Внимание!"), 
					QString::fromUtf8("Границы разметки превышают границы файла, разметка будет обрезана по длинне файла, загрузить разметку?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) )
			{
				phrasesList->GetData().SetPhrases(data);
				settings.setValue("lastMarkingFileName", fileName);
				return true;
			}
			else 
			{
				if ( !data.size() )
					QMessageBox::information(this, QString::fromUtf8("Внимание"),
							QString::fromUtf8("Файл \"%1\" не содержит разметок!!!").arg(fileName));

				controller->RemoveList(phrasesList);
				foreach(Phrase * phrase, data)
					delete phrase;
			}
		}
	}
	return false;
}

bool MainWindow::SaveMarking(const QString & fileName,  const QList<Phrase*> & marking)
{
	if ( WaveViewerWidget * widget = ui.tabWidget->GetCurWaveWidget() )
	{
		QFile file(fileName);
		if ( file.open(QFile::WriteOnly) )
		{
			foreach(Phrase * phrase, marking)
			{
				qulonglong start = widget->ToSec(phrase->GetStart()) * 10000000.;
				qulonglong end = widget->ToSec(phrase->GetEnd()) * 10000000.;
				QString str = QString("%1 %2 %3\r\n").arg(start).arg(end).arg(phrase->GetName());
				
				if( settingsDialog->OpenMarkingAsUTF8() )
					file.write(str.toUtf8());
				else
					file.write(str.toLocal8Bit());

			}
			settings.setValue("lastMarkingFileName", fileName);
			return true;
		}
	}
	return false;
}

void MainWindow::on_action_showAboutDilaog_triggered(bool checked)
{
	AboutDialog dialog;
	dialog.exec();
}

void MainWindow::on_action_signal_desc_save_as_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		QFileInfo info(view->GetFileName());

		QString fileName = QFileDialog::getSaveFileName(this, QString::fromUtf8("Сохранить описание сигнала в файл"),
														info.absolutePath() + "/" + info.completeBaseName() + ".json",
														 QString::fromUtf8("Описание (*.JSON *.json)"));
		if ( !fileName.isEmpty() )
		{
			signal_desc_frame->SaveToFile(fileName);
			view->SetSignalDescFilePath(fileName);
		}
	}
}

void MainWindow::on_action_signal_desc_save_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		if ( !view->GetSignalDesc().first.isEmpty() )
			signal_desc_frame->SaveToFile(view->GetSignalDesc().first);
		else
			on_action_signal_desc_save_as_triggered(true);
	}
}

void MainWindow::on_action_open_opened_files_dialog_triggered(bool checked)
{
	OpenedFilesDialog dialog(*this);
	dialog.Exec();
}


void MainWindow::on_action_signal_desc_load_triggered(bool checked)
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		QFileInfo info(view->GetSignalDesc().first.isEmpty() ? view->GetFileName() : view->GetSignalDesc().first);
		QString file = QFileDialog::getOpenFileName(this,
									QString::fromUtf8("Открыть файл описания сигнала"),
									info.absolutePath() + "/" + info.completeBaseName() + ".json",
									QString::fromUtf8("Описание (*.JSON *.json)"));
		if ( !file.isEmpty() )
		{
			if ( signal_desc_frame->LoadFromFile(file) )
			{
				view->SetSignalDescFilePath(file);
				view->SetSignalDesc(signal_desc_frame->GetDesc());
			}
		}
	}
}

void MainWindow::OnSignalDescChanged()
{
	if ( FileView * view = ui.tabWidget->GetCurFileView() )
	{
		qDebug() << "OnSignalDescChanged";
		view->SetSignalDesc(signal_desc_frame->GetDesc());

		if ( settingsDialog->IsAutoSaveSignalDesc() )
			on_action_signal_desc_save_triggered(true);
	}
}