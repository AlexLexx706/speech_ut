#include "audiooutputds.h"
#include "internalautputdevice/internalautputdevice.h"


AudioOutputDS::AudioOutputDS(const QAudioFormat & format, QObject * parent):QObject(parent)
{
	d = new InternalAutputDevice(format);
	connect(d, SIGNAL(notify()), this, SIGNAL(notify()));
	connect(d, SIGNAL(stateChanged(QAudio::State)), this, SIGNAL(stateChanged(QAudio::State)));
}
AudioOutputDS::~AudioOutputDS()
{
	delete d;
}

QAudio::Error AudioOutputDS::error() const
{
	return d->error();
}

QAudioFormat AudioOutputDS::format() const
{
	return d->format();
}

int	AudioOutputDS::notifyInterval() const
{
	return d->notifyInterval();
}

void AudioOutputDS::setNotifyInterval(int ms)
{
	d->setNotifyInterval(ms);
}

qint64 AudioOutputDS::processedUSecs() const
{
	return d->processedUSecs();
}

QAudio::State AudioOutputDS::state() const
{
	return d->state();
}


void AudioOutputDS::reset()
{
	d->reset();
}

void AudioOutputDS::resume()
{
	d->resume();
}


void AudioOutputDS::start(const QByteArray & data)
{
	d->start(data);
}

void AudioOutputDS::stop()
{
	d->stop();
}

void AudioOutputDS::suspend()
{
	d->suspend();
}

quint32  AudioOutputDS::GetFrequency()
{
	return d->GetFrequency();
}

void AudioOutputDS::SetFrequency(quint32 f)
{
	d->SetFrequency(f);
}

quint32 AudioOutputDS::GetCurPos()
{
	return d->GetCurPos();
}

void AudioOutputDS::SetCurPos(quint32 pos)
{
	d->SetCurPos(pos);
}
