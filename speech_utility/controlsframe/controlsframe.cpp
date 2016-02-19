#include "controlsframe.h"
#include <QRegExp>
#include <math.h>
#include "../mainwindow.h"

ControlsFrame::ControlsFrame(QWidget *parent):
	QFrame(parent),
	type(Samples),
	sampleRate(8000)
{

}

ControlsFrame::~ControlsFrame()
{

}

void ControlsFrame::SetFormatType(FormatType _type)
{
	if ( type != _type )
	{
		type = _type;
		OnSelectedChanged(GetSelectedStart(), GetSelectedEnd());
		OnPlayPosChanged(GetPlayPos());
		OnPhraseChanged(GetPhraseStart(), GetPhraseEnd(), GetPhraseName());
	}
}

void ControlsFrame::SetSampleRate(int sm)
{
	if (sm != sampleRate )
	{
		sampleRate = sm;
		OnSelectedChanged(GetSelectedStart(), GetSelectedEnd());
		OnPlayPosChanged(GetPlayPos());
		OnPhraseChanged(GetPhraseStart(), GetPhraseEnd(), GetPhraseName());
	}
}

long ControlsFrame::GetSelectedStart(){return 0;};
long ControlsFrame::GetSelectedEnd(){return 0;};
long ControlsFrame::GetPlayPos(){return 0;};
long ControlsFrame::GetPhraseStart(){return 0;};
long ControlsFrame::GetPhraseEnd(){return 0;};
QString ControlsFrame::GetPhraseName(){return "";};

void ControlsFrame::OnSelectedChanged(long startSample, long endSample){};
void ControlsFrame::OnPlayPosChanged(long samples){};
void ControlsFrame::OnMarkingChanged(const QString & markingName, 
					bool pahrasesIntersecton,
					const QColor & color,
					const QString & fileName,
					const QString & maringStr){};
void ControlsFrame::OnPhraseChanged(long startSample, long endSample, const QString & name){};

void ControlsFrame::OnPlayChanged(bool play){};
void ControlsFrame::SetEnabledPlayback(bool enable){};
bool ControlsFrame::IsPlaybackEnabled(){return false;}

void ControlsFrame::SetEnabledPhrase(bool enabled){};
bool ControlsFrame::IsPhraseEnabled(){return false;};

void ControlsFrame::SetPhraseNameFocus(){};

void ControlsFrame::SetEnabledMarking(bool enabled){};
bool ControlsFrame::IsMarkingEnabled(){return false;};






QString ControlsFrame::FromSampleToSecStr(long sample)
{
	return MainWindow::FromSecToStrign(sample/(qreal)sampleRate);
}

long ControlsFrame::FromSecStrToSample(const QString & time)
{
	qreal sec;

	if (MainWindow::FromStringToSec(time, sec))
		return sec * sampleRate;
	return 0;
}
