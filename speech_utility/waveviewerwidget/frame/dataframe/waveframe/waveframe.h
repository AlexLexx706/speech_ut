#ifndef WAVE_FRAME_A_H
#define WAVE_FRAME_A_H
#include "../dataframe.h"

//! ������������ ������.
class WaveFrame:public DataFrame
{
public:
	WaveFrame(QByteArray & data);

protected:
	//! ������ �����.
	QSize pointSize;

	virtual void UpdateBorders();
	virtual void Update();
	virtual void CheckDataReady(){isReady = GetCount();}
};

#endif // WAVE_FRAME_A_H
