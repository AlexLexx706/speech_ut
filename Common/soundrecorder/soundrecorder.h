/*******************************************************************************

Имя файла:		@file		soundrecorder.h
Описание файла:	@brief		Определение класса записи сырого звука в wav файл.
Модуль:			@package	Player
Тип модуля:		Собственная разработка
Авторы:			@author		Калмыков А.В.

*******************************************************************************/
#ifndef __SOUND_RECORDER_ALEX__
#define __SOUND_RECORDER_ALEX__
#include "../common_global.h"

class QString;
class QIODevice;
class QMutex;
class QByteArray;

//! Класс записывает звуковые данные в файл.
class COMMON_EXPORT SoundRecorder
{
public:
	typedef unsigned short SHORT;
	typedef unsigned long DWORD;
	typedef unsigned char BYTE;

	#define  NAME_SIZE 4
	#define PCM 1

	//! Описание структур wave файла.
	#pragma pack(1)

	//! Кусок.
	struct Chunk
	{
		Chunk()
		{
			name[0] = 0;
			name[1] = 0;
			name[2] = 0;
			name[3] = 0;
			size = 0;
		}

		BYTE name[NAME_SIZE];
		DWORD size;
	};

	//! Описание файла.
	struct WaveDesc:public Chunk
	{
		WaveDesc()
		{
			name[0] = 'R';
			name[1] = 'I';
			name[2] = 'F';
			name[3] = 'F';
			size = 4;	

			wave[0] = 'W';
			wave[1] = 'A';
			wave[2] = 'V';
			wave[3] = 'E';
		}
		BYTE wave[NAME_SIZE];
	};

	//! Описание формата.
	struct WaveFormat:public Chunk
	{
		WaveFormat()
		{
			name[0] = 'f';
			name[1] = 'm';
			name[2] = 't';
			name[3] = ' ';
			size = 16;

			format = 0;
			channels = 0;
			sampleRate = 0;
			byteRate = 0;
			blockAlign = 0;
			bitsPerSample = 0;
		}

		SHORT format;
		SHORT channels;
		DWORD sampleRate;
		DWORD byteRate;
		SHORT blockAlign;
		SHORT bitsPerSample;
	};

	struct DataChunk:public Chunk
	{
		DataChunk()
		{
			name[0] = 'd';
			name[1] = 'a';
			name[2] = 't';
			name[3] = 'a';
		}
	};
	#pragma pack()


	//! Конструктор.
	SoundRecorder();

	//! Деструктор.
	~SoundRecorder();

	/** 
	 * Установить канал для записи звука.
	 * @param channels [in] - количество каналов.
	 * @param sampleRate [in] - количество отсчётов в секунду.
	 * @param bitsPerSample [in] - число бит на отсчёт.
	 * @return - true - всё пучком.
	*/
	bool CreateWaveFile(QIODevice * device, 		
				SHORT channels,
				DWORD sampleRate,
				SHORT bitsPerSample);

	//! Возвращает true если файл создан для записи.
	bool IsOpen();
	
	/** 
	 * Закрыть файл.
	 * @return - true - всё пучком.
	*/
	bool CloseFile();

	/**
	 * Записать данные в файл.
	 * @param data [in] - буффер звук.
	 * @param size [in] - размер буффера звука в байтах.
	 * @return - true - всё пучком.
	*/
	bool WriteSound(const BYTE * data, DWORD size);

	/**
	 * Записать данные в файл.
	 * @param data [in] - буффер звук.
	 * @param size [in] - размер буффера звука в байтах.
	 * @return - true - всё пучком.
	*/
	bool WriteSound(const QByteArray & data);

	//! Записать wave файл.
	static bool WriteWaveFile(QIODevice * device, 
							SHORT channels,
							DWORD sampleRate,
							SHORT bitsPerSample,
							const QByteArray & inData);

	//! Прочитать wave файл.
	static bool ReadWaveFile(QIODevice * device, WaveFormat & format, QByteArray & outData);

private:
	//! Описание.
	WaveDesc desc;

	//! Формат.
	WaveFormat format;

	//! Дата.
	Chunk data;

	//! Файл.
	QIODevice * device;

	//! Для синхронизации.
	QMutex * mutex;

	//! Количество записанныз байт.
	DWORD writeSize;

	//! Сброс параметров.
	void ResetParams();
};

#endif	//__SOUND_RECORDER_ALEX__