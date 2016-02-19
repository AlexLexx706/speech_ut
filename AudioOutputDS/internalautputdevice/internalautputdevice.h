#ifndef INTERNALAUTPUTDEVICE_H
#define INTERNALAUTPUTDEVICE_H

#include <QObject>
#include <QIODevice>
#include <QTimer>
#include <QAudioFormat>
#include <QAudio>
#include "DXUTsound.h"


class InternalAutputDevice:public QObject
{
	Q_OBJECT
public:
	InternalAutputDevice(const QAudioFormat & format);
	~InternalAutputDevice();

	QAudio::Error error() const {return curError;};
	QAudioFormat format() const {return curFormat;}
	int	 notifyInterval() const {return timer.interval();}
	void setNotifyInterval(int ms) { timer.setInterval(ms);}
	QAudio::State state() const {return curState;}
	qint64 processedUSecs() const;

	void resume();
	void suspend();
	void start(const QByteArray & data);
	void stop();
	void reset();
	quint32  GetFrequency();
	void SetFrequency(quint32 f);

	quint32 GetCurPos();
	void SetCurPos(quint32 pos);



signals:
	void notify();
	void stateChanged(QAudio::State state);

private:
	QAudioFormat curFormat;
	QAudio::Error curError;
	QAudio::State curState;
	QByteArray data;
	CSoundManager * g_pSoundManager;
	CSound * g_pSound;
	QTimer timer;
	quint32 freq;
	HWND wnd;
private slots:
	void OnNotify();

};


#endif // INTERNALAUTPUTDEVICE_H
