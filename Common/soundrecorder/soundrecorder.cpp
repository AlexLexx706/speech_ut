/*******************************************************************************

Имя файла:		@file		soundrecorder.cpp
Описание файла:	@brief		Реализация класса записи сырого звука в wav файл.
Модуль:			@package	Player
Тип модуля:		Собственная разработка
Авторы:			@author		Калмыков А.В.

*******************************************************************************/

#include "soundrecorder.h"
#include <string.h> 
#include <QString>
#include <QIODevice>
#include <QMutex>
#include <QByteArray>
#include <QDebug>

SoundRecorder::SoundRecorder():
	device(NULL), 
	mutex( new QMutex(QMutex::Recursive))
{
	ResetParams();
}

void SoundRecorder::ResetParams()
{
	writeSize = 0;

	//Описание RIFF.
	memcpy(desc.name, "RIFF", sizeof(desc.name));
	desc.size = 4;
	memcpy(desc.wave, "WAVE", sizeof(desc.wave));


	//Описание fmt .
	memset(&format, 0, sizeof(format));
	memcpy(format.name, "fmt ", sizeof(format.name));
	format.size = 16;

	//Описание data
	memcpy(data.name, "data", sizeof(data.name));
	data.size = 0;	
}

SoundRecorder::~SoundRecorder()
{
	CloseFile();
	delete mutex;
}


bool SoundRecorder::IsOpen()
{
	QMutexLocker ml(mutex);
	return (bool)device;
}

bool SoundRecorder::CreateWaveFile(QIODevice * _device,
				SHORT channels,
				DWORD sampleRate,
				SHORT bitsPerSample)
{
	QMutexLocker ml(mutex);

	CloseFile();

	if ( _device &&
		channels > 0 &&
		sampleRate > 0 &&
		bitsPerSample > 0 )
	{
		device = _device;
		ResetParams();

		//Запомнили формат.
		format.format = PCM;
		format.channels = channels;
		format.sampleRate = sampleRate;
		format.bitsPerSample = bitsPerSample;
		format.byteRate = format.sampleRate * format.channels * format.bitsPerSample/8;
		format.blockAlign = format.channels * format.bitsPerSample/8;
		format.size = 16;

		if ( device->open(QIODevice::WriteOnly) )
		{
			//Записываем Данные в файл.
			//1. desc
			device->write((const char *)&desc, sizeof(desc));
			
			//2.format
			device->write((const char *)&format, sizeof(format));
			
			//3. data
			device->write((const char *)&data, sizeof(data));
			return true;
		}
		else
		{
			qWarning() << "CreateWaveFile error:" << device->errorString();
		}
		CloseFile();
	}
	return false;
}

bool SoundRecorder::WriteWaveFile(QIODevice * device, 
							SHORT channels,
							DWORD sampleRate,
							SHORT bitsPerSample,
							const QByteArray & inData)
{
	SoundRecorder temp;
	return temp.CreateWaveFile(device, channels, sampleRate, bitsPerSample) && 
			temp.WriteSound(inData) && 
			temp.CloseFile();
}

//! Прочитать wave файл.
bool SoundRecorder::ReadWaveFile(QIODevice * device, WaveFormat & format, QByteArray & outData)
{
	if ( device )
	{
		outData.clear();
		memset(&format, 0, sizeof(format));

		if ( device->open(QIODevice::ReadOnly) )
		{
			WaveDesc desc;

			//Читаем данные.
			//1. desc
			if ( device->read((char *)&desc, sizeof(desc)) == sizeof(desc) )
			{
				//! Проверим размер данных.
				Chunk data;

				if ( (desc.size - NAME_SIZE) >= (sizeof(format) + sizeof(data))  )
				{
					//2.format
					if ( device->read((char *)&format, sizeof(format)) == sizeof(format) )
					{
						//3. data
						if ( device->read((char *)&data, sizeof(data)) == sizeof(data) )
						{
							//4. Читаем данные.
							outData = device->read(device->bytesAvailable()
									/*device->bytesAvailable() > data.size ? data.size: device->bytesAvailable()*/);
							return outData.size();
						}
					}
				}
			}
		}
	}
	return false;
}
	
bool SoundRecorder::CloseFile()
{
	QMutexLocker ml(mutex);

	if ( device )
	{
		//Проставим павильные значения размеров Chunks.
		//1. Общий размер.
		device->seek(NAME_SIZE);
		DWORD size = sizeof(desc.wave) + sizeof(format) + sizeof(data) + writeSize;
		device->write((const char *)&size, sizeof(size));

		//2. Размер данных.
		device->seek(sizeof(desc) + sizeof(format) + NAME_SIZE);
		size = writeSize;
		device->write((const char *)&size, sizeof(size));

		//3. Закроем файл.
		device->close();
		device = NULL;
		return true;
	}
	return false;
}

bool SoundRecorder::WriteSound(const BYTE * data, DWORD size)
{
	QMutexLocker ml(mutex);
	if ( device )
	{
		qint64 writen;

		do 
		{
			writen = device->write((const char *)data, size);
			if (writen == -1 )
			{
				qWarning() << "WriteSound error:" << device->errorString();			
				return false;
			}
			writeSize += writen;
			size -= writen;
		}
		while (size);
		return true;
	}
	return false;
}


bool SoundRecorder::WriteSound(const QByteArray & data)
{
	return WriteSound((const BYTE *)data.constData(), data.size());
}
