#ifndef AUDIOOUTPUTDS_H
#define AUDIOOUTPUTDS_H

#include "audiooutputds_global.h"
#include <QObject>
#include <QAudioFormat>
#include <QAudio>
#include <QTimer>

class InternalAutputDevice;
class QByteArray;

class AUDIOOUTPUTDS_EXPORT AudioOutputDS:public QObject
{
	Q_OBJECT

public:
	AudioOutputDS(const QAudioFormat & format = QAudioFormat(), QObject * parent = 0);
	
	~AudioOutputDS();
	
	QAudio::Error error() const;
	
	QAudioFormat format() const;
	
	int	notifyInterval() const;
	
	void	setNotifyInterval(int ms);
	
	qint64	processedUSecs() const;
	
	QAudio::State	state() const;

	void	reset();
	
	void	resume();

	void	start(const QByteArray & data);
	
	void	stop();
	
	void	suspend();
	
	quint32  GetFrequency();
	
	void SetFrequency(quint32 f);
	
	quint32 GetCurPos();
	
	void SetCurPos(quint32 pos);

signals:
	void notify();
	void stateChanged(QAudio::State state);

private:
	InternalAutputDevice * d;
};


#endif // AUDIOOUTPUTDS_H
