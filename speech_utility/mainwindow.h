#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <qmainwindow.h>
#include <QAudioOutput>
#include <QBuffer>
#include <QAudioDeviceInfo>
#include <QShortcut>
#include <QSettings>
#include "../BaseSUPlugin/plugincommon.h"
#include <QThread>
#include <QLibrary>
#include "../AudioOutputDS/audiooutputds.h"
#include "phrasedeldiaog/phrasedeldiaog.h"
#include "../Common/soundrecorder/soundrecorder.h"
#include "speech_utility_global.h"
#include "QFile"
#include "QTextStream"


class WaveViewerWidget;
class PhraseController;
class PhrasesListData;
class PhrasesList;
class Phrase;
class SearchDialog;
class FileView;
class QPluginLoader;
class SettingsDialog;
class QAbstractButton;
class FileViewController;
class ControlsFrame;
class SignalDescFrame;

class SPEECH_UTILITY_EXPORT MainWindow : public QMainWindow
{
	Q_OBJECT
	friend class SettingsDialog;

public:
	//! Режим использования приложения.
	enum UsignMode
	{
		Normal,
		ConsoleCutVad,
		CutVad,
		ExtNoEdit,
		ExtEdit
	};

	MainWindow(const QVariantMap & settings, QWidget *parent = 0);
	~MainWindow();

	FileViewController &  GetFVC();

	//void SetUsignMode(UsignMode mode);
	bool CreateAudioOutput(const SoundRecorder::WaveFormat & soundFormat);
	bool SetSettings(const QString & settings);
	static QString FromSecToStrign(qreal t);
	static bool FromStringToSec(const QString & time, qreal & res);


private slots:

/////////////////////////////////////////////////////
	void onStartSelectedChanged(long value);
	void onEndSelectedChanged(long value);

	void onMarkingNameChanged(const QString & name);
	void onMarkingIntersectionChanged(bool state);
	void onMarkingColorChanged(const QColor & color);
	void onMoveMarkingUp();
	void onMoveMarkingDown();

	void onStartPhraseChanged(long value);
	void onEndPhraseChanged(long value);
	void onPhraseNameChanged(const QString & name);

	void onPlaybackChanged(bool play);
	void onPauseClicked();
/////////////////////////////////////////////////////


	void on_action_exit_triggered(bool checked);
	void on_action_openFile_triggered(bool checked);
	void on_action_showAll_triggered(bool checked);

	void on_action_addPhrase_triggered(bool checked);
	void on_action_delPhrase_triggered(bool checked);
	void on_action_copyFrame_triggered(bool checked);
	void on_action_delFrame_triggered(bool checked);
	void on_action_insertFrame_triggered(bool checked);
	void on_action_selectAll_triggered(bool checked);
	void on_action_cutPhrase_triggered(bool checked);
	void on_action_showSample_triggered(bool checked);
	void on_action_showTime_triggered(bool checked);
	void on_action_addMarking_triggered(bool checked);
	void on_action_delMarking_triggered(bool checked);
	
	
	void on_action_addMarkingFormFile_triggered(bool checked);
	void on_action_saveMarking_triggered(bool checked);
	void on_action_saveMarkingAs_triggered(bool checked);

	void on_action_searchPhrase_triggered(bool checked);
	void on_action_closeFile_triggered(bool checked);
	void on_action_closeAllFiles_triggered(bool checked);
	void on_action_setupGradient_triggered(bool checked);

	void on_action_showPluginFrame_triggered(bool checked);
	void on_action_hidePluginFrame_triggered(bool checked);
	void on_action_clearPluginMessages_triggered(bool checked);
	void on_action_openSettings_triggered(bool checked);
	void on_action_editPhraseText_triggered(bool checked);
	void on_action_moveLeft_leftMarkerBorder_triggered(bool checked);
	void on_action_moveRight_leftMarkerBorder_triggered(bool checked);
	void on_action_moveLeft_rightMarkerBorder_triggered(bool checked);
	void on_action_moveRight_rightMarkerBorder_triggered(bool checked);
	void on_action_saveFileAs_triggered(bool checked);
	void on_action_save_file_triggered(bool checked);


	void on_action_removePhrasesByName_triggered(bool checked);
	void on_action_copyPluginMessages_triggered(bool checked);

	void on_action_zoom_in_vertical_triggered(bool checked);
	void on_action_zoom_out_vertical_triggered(bool checked);
	void on_action_zoom_in_horizontal_triggered(bool checked);
	void on_action_zoom_out_horizontal_triggered(bool checked);


	void on_action_selectNextMarking_triggered(bool checked);
	void on_action_cutFile_triggered(bool checked);

	void on_action_nextPage_triggered(bool checked);
	void on_action_previousPage_triggered(bool checked);
	void on_action_removeUnmarks_triggered(bool checked);

	void on_action_showAboutDilaog_triggered(bool checked);
	void on_action_PrepareForTrans_triggered(bool checked);

	void on_action_signal_desc_save_as_triggered(bool checked);
	void on_action_signal_desc_save_triggered(bool checked);
	void on_action_signal_desc_load_triggered(bool checked);
	void on_action_open_opened_files_dialog_triggered(bool checked);



///////////////////////////////////////////////////////
	void OnGetMarkingTriggered();
	void OnGetMarkingOnMarkingTriggered();

	void OnStartBatchProcessingTriggered();
	void OnStopBatchProcessingTriggered();

	void OnMakeMarkingStarted(int ID);
	void OnMakeMarkingFinished(int id, const SUPOutData & data, bool result, const QString & errorDesc);

	void OnBatchProcessingStarted(int ID);
	void OnBatchProcessingFinished(int ID, bool result, const QString & errorDesc);
	void OnProgeressMessage(int ID, qreal progress, const QString & message);
	void OnFileProcessingResult(int id, const SUPFileResult & result);
	void OnStartFileProcessing(int id, const QString & fileName);

	void OnPhraseDoubleClick();
	void OnCurrentListChanged();
	void OnPhraseDataChanged();
	void OnFindNext();
	void OnChangeType();
	void OnCurViewChanged();
	void OnDropWavFile(const QString &);
	void OnDropMarkingFile(const QString &);

	void OnSelectedSamplesChanged(long startFrame, long endFrame);
	void OnNotify();
	void OnStateChanged(QAudio::State);
	void OnPosChanged(long sample, double amplitude);
	void OnCurrentPhraseChanged();
	void OnSignalDescChanged();

	void onDataChanged(long);

private:
	struct TempPluginData
	{
		FileView * View;
	};

	void * _ui;
	SoundRecorder::WaveFormat soundFormat;
	AudioOutputDS * audioOutput;
	long offset;
	QByteArray copyData;
	QSettings settings;
	SearchDialog * searchDialog;
	QList< QPair< SUPluginInterface *, QPluginLoader * >  > pluginsList;
	QMap<int, TempPluginData> tempPluginDataInt;
	int pluginTaskCounter;
	int pluginProgressCount;
	int messageCounter;
	SettingsDialog * settingsDialog;
	UsignMode curMode;
	QString outFileName;
	//минимальная длинна речи.
	float minSpeechDur;
	bool useDictorCount;
	ControlsFrame * controlsFrame;
	bool makeXml;
	QFile logFile;
	QTextStream logStream;
	SignalDescFrame * signal_desc_frame;

	void UpdatePhraseData(bool checkWindow);
	void LoadPlugins(const QString & path);
	void RemoveAllPlugins();
	bool Play();
	bool Stop();
	bool IsPlayed();
	void UpdatePlayPos(long samples);

	bool OpenFile(const QString & fileName);
	bool OpenFilesAsOne(const QStringList & files);
	bool SaveFile(const QString & fileName);
	bool SaveFile(const QString & fileName, 
				const QByteArray & data,
				int sampleRate,
				 int bitPersample);
	bool LoadMarking(const QString & fileName);
	bool SaveMarking(const QString & fileName,  const QList<Phrase*> & marking);

	void GetAllActions(QList<QAction *> & list);
	void GetAllButtons(QList<QAbstractButton *> & list);

	bool StartMakeMarking(SUPluginInterface * plugin, const QString & params);
	bool StartMakeMarkingOnMarking(SUPluginInterface * plugin, const QString & params);
	SUPluginInterface * GetPluginByName(const QString & name);
	bool RemovePhraseByType(PhrasesListData & list, Phrase * phrase, PhraseDelDialog::ResultCode code);
	void CheckPhrasesDuraton();
	void SelestFirstPhrase(PhraseController * controller, PhrasesList * list);

	void SetupControlsFrame(ControlsFrame * frame);
	void AddPluginMessage(const QString & message);
};

#endif //_MAIN_WINDOW_H_