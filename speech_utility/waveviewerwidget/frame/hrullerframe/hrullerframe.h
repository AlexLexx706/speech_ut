#ifndef HRULLER_FRAME_A_H
#define HRULLER_FRAME_A_H
#include "../vrullerframe/vrullerframe.h"
#include "../../../speech_utility_global.h"


//!Рисунок горизонтальной линейки.
class SPEECH_UTILITY_EXPORT HRullerFrame:public VRullerFrame
{
public:
	HRullerFrame():sampleRate(8000){}

	virtual void SetWindow(const QRectF & window);
	void SetSampleRate(int rate);
protected:
	int sampleRate;

	virtual void Update();
};

#endif // HRULLER_FRAME_A_H
