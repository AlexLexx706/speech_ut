#ifndef DATA_FRAME_A_H
#define DATA_FRAME_A_H
#include "../frame.h"
#include <QByteArray>
#include "../../../speech_utility_global.h"

//! ������� ������.
class SPEECH_UTILITY_EXPORT DataFrame:public Frame
{
	Q_OBJECT
public:
	DataFrame(QByteArray & data);

	//! ���������� ������
	bool SetData(const QByteArray & data, int _bitsPerSample, int _sampleRate);

	//! �������� ������.
	bool GetData(QByteArray & data, long startSample, long count) const;

	const QByteArray & GetData() const {return data;}

	//! ������� ������.
	bool RemoveData(long startSample, long count);

	//! �������� ������.
	bool InsertData(const QByteArray & data, long startSample);

	//! ���������� ������� � ������.
	inline int GetCount() const { return data.size()/(bitsPerSample>>3); }

	//! ������� �������������.
	inline int GetSampleRate() const { return sampleRate; }

	//! ��� �� ������.
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

	//! �������� �������.
	virtual void UpdateBorders(){};

	virtual void BeforeChange(){};
	
	virtual void AfterChange(){isReady = true;};

	virtual void CheckDataReady(){};
};

#endif // DATA_FRAME_A_H
