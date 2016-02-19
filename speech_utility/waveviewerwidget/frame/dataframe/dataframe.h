#ifndef DATA_FRAME_A_H
#define DATA_FRAME_A_H
#include "../frame.h"
#include <QByteArray>
#include "../../../speech_utility_global.h"

//! рисунок данных.
class SPEECH_UTILITY_EXPORT DataFrame:public Frame
{
	Q_OBJECT
public:
	DataFrame(QByteArray & data);

	//! Установить данные
	bool SetData(const QByteArray & data, int _bitsPerSample, int _sampleRate);

	//! Получить данные.
	bool GetData(QByteArray & data, long startSample, long count) const;

	const QByteArray & GetData() const {return data;}

	//! Удалить данные.
	bool RemoveData(long startSample, long count);

	//! Вставить данные.
	bool InsertData(const QByteArray & data, long startSample);

	//! Количество семплов в данных.
	inline int GetCount() const { return data.size()/(bitsPerSample>>3); }

	//! Частота дискретизации.
	inline int GetSampleRate() const { return sampleRate; }

	//! Бит на отсчёт.
	inline int GetBitPersample() const { return bitsPerSample; }

	inline QRectF GetMaxWindow() const { return maxWindow;}

	inline QSizeF GetMinWindowSize() const { return minWindowSize; }

signals:
	void DataChanged(long count);

protected:
	QByteArray & data;
	int sampleRate;
	int bitsPerSample;
	QRectF maxWindow;
	QSizeF minWindowSize;

	//! Обновить границы.
	virtual void UpdateBorders(){};

	virtual void BeforeChange(){};
	
	virtual void AfterChange(){isReady = true;};

	virtual void CheckDataReady(){};
};

#endif // DATA_FRAME_A_H
