#ifndef VRULLER_FRAME_H
#define VRULLER_FRAME_H
#include "../frame.h"
#include <QVector>
#include "../../../speech_utility_global.h"

//!Рисунок вертикальной линейки.
class SPEECH_UTILITY_EXPORT VRullerFrame:public Frame
{
public:
	VRullerFrame(): cellWidth(60), showSamples(true){}

	virtual void SetWindow(const QRectF & window);

	void SetCelWidth(int _cellWidth);

	inline int GetCellWidth() const {return cellWidth; }

	void ShowSamples(bool samples);

	bool IsShowSamples() const { return showSamples;}

	inline QVector<int> GetMarks() const {return marks;}

protected:
	int cellWidth;
	bool showSamples;
	QVector<int> marks;

	virtual void Update();
};

#endif // VRULLER_FRAME_H
