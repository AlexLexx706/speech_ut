#ifndef CONTROLS_H
#define CONTROLS_H

#include <QFrame>
#include "../speech_utility_global.h"

class SPEECH_UTILITY_EXPORT ControlsFrame : public QFrame
{
	Q_OBJECT

public:
	enum FormatType
	{
		Samples,
		Time
	};

	ControlsFrame(QWidget *parent);
	~ControlsFrame();

	void SetFormatType(FormatType type);
	FormatType GetFormatType() const { return type;}
	
	void SetSampleRate(int sm);
	int GetSampleRate() const {return sampleRate;}

	virtual long GetSelectedStart();
	virtual long GetSelectedEnd();
	virtual long GetPlayPos();
	virtual long GetPhraseStart();
	virtual long GetPhraseEnd();
	virtual QString GetPhraseName();
	virtual void SetConfig(const QString & jsonString){};


public slots:
	virtual void OnSelectedChanged(long startSample, long endSample);
	virtual void OnPlayPosChanged(long samples);
	virtual void OnMarkingChanged(const QString & markingName, 
									bool pahrasesIntersecton,
									const QColor & color,
									const QString & fileName,
									const QString & maringStr);
	virtual void OnPhraseChanged(long startSample, long endSample, const QString & name);
	
	virtual void OnPlayChanged(bool play);
	virtual void SetEnabledPlayback(bool enable);
	virtual bool IsPlaybackEnabled();
	
	virtual void SetEnabledPhrase(bool enabled);
	virtual bool IsPhraseEnabled();

	virtual void SetPhraseNameFocus();

	virtual void SetEnabledMarking(bool enabled);
	virtual bool IsMarkingEnabled();


signals:
	void StartSelectedChanged(long value);
	void EndSelectedChanged(long value);

	void MarkingNameChanged(const QString & name);
	void MarkingIntersectionChanged(bool state);
	void MarkingColorChanged(const QColor & color);
	void MoveMarkingUp();
	void MoveMarkingDown();

	void StartPhraseChanged(long value);
	void EndPhraseChanged(long value);
	void PhraseNameChanged(const QString & name);

	void PlaybackChanged(bool play);
	void PauseClicked();

protected:
	QString FromSampleToSecStr(long sample);
	long FromSecStrToSample(const QString & str);

private:
	FormatType type;
	int sampleRate;

};

#endif // CONTROLS_H
