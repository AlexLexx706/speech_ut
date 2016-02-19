#ifndef EXT_CONTROLS_H
#define EXT_CONTROLS_H

#include "../controlsframe.h"
#include "../../speech_utility_global.h"

class SPEECH_UTILITY_EXPORT ExtControlsFrame : public ControlsFrame
{
	Q_OBJECT

public:
	ExtControlsFrame(QWidget *parent);
	~ExtControlsFrame();

public slots:
	long GetSelectedStart();
	long GetSelectedEnd();
	long GetPlayPos();
	long GetPhraseStart();
	long GetPhraseEnd();
	QString GetPhraseName();

	void OnSelectedChanged(long startSample, long endSample);
	void OnPlayPosChanged(long samples);
	void OnMarkingChanged(const QString & markingName, 
							bool pahrasesIntersecton,
							const QColor & color,
							const QString & fileName,
							const QString & maringStr);
	void OnPhraseChanged(long startSample, long endSample, const QString & name);
	
	void OnPlayChanged(bool play);
	void SetEnabledPlayback(bool enable);
	bool IsPlaybackEnabled();

	void SetEnabledPhrase(bool enabled);
	bool IsPhraseEnabled();

	void SetPhraseNameFocus();

	void SetEnabledMarking(bool enabled);
	bool IsMarkingEnabled();

private slots:
	void on_lineEdit_start_editingFinished();
	void on_lineEdit_end_editingFinished();
	void on_lineEdit_len_editingFinished();
	void on_lineEdit_markingName_editingFinished();

	void on_checkBox_markingIntersection_toggled(bool checked);
	void on_pushButton_setColor_clicked();

	void on_lineEdit_start_phrase_editingFinished();
	void on_lineEdit_end_phrase_editingFinished();
	void on_lineEdit_len_phrase_editingFinished();
	void on_lineEdit_phrase_name_editingFinished();
	void on_pushButton_playStop_toggled(bool checked);



private:
	void * _ui;
	QColor color;

};

#endif // EXT_CONTROLS_H
