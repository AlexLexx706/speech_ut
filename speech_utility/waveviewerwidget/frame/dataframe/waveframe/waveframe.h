#ifndef WAVE_FRAME_A_H
#define WAVE_FRAME_A_H
#include "../dataframe.h"

//! Асцилограмма данных.
class WaveFrame:public DataFrame
{
public:
	WaveFrame(QByteArray & data);

protected:
	//! Размер точек.
	QSize pointSize;

	virtual void UpdateBorders();
	virtual void Update();
	virtual void CheckDataReady(){isReady = GetCount();}
};

#endif // WAVE_FRAME_A_H
