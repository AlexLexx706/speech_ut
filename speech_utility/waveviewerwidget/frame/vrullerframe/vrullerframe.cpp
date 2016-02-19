#include "vrullerframe.h"
#include <assert.h>
#include <QPainter>

//////////////////////////////////////
void VRullerFrame::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		if ( window.height() != _window.height() || window.top() != _window.top() )
		{
			window = _window;
			Update();
		}
		else 
			window = _window;
	}
}

void VRullerFrame::SetCelWidth(int _cellWidth)
{
	if (_cellWidth != cellWidth )
	{
		cellWidth = _cellWidth;
		Update();
	}
}

void VRullerFrame::ShowSamples(bool _samples)
{
	if ( showSamples != _samples )
	{
		showSamples = _samples;
		Update();
	}
}


void VRullerFrame::Update()
{
	if ( !pixmap.isNull() )
	{
		//Горизонтальные линии.
		QPainter painter(&pixmap);
		pixmap.fill(Qt::gray);

		qreal dy = pixmap.height()/window.height();

		int hCount = pixmap.height()/cellWidth;
		qreal hStep = window.height()/hCount;
		if ( hStep < 1.) hStep = 1.;
		qreal start = ((int)(window.y()/hStep)) * hStep;
		qreal hStart = start - window.y();
		hCount = hCount+2;

		painter.setPen(Qt::white);
		painter.drawLine(0, 0, 0, pixmap.height());

		marks.resize(hCount);
		for (int i = 0; i < hCount; i++ )
		{
			qreal temp = hStep * i;
			qreal y = pixmap.height() - (hStart + temp) * dy;
			marks[i] = y;
			painter.drawLine(0, y, 3, y);
			painter.drawText(QRect(6, y - painter.fontMetrics().height()/2, pixmap.width() - 6, painter.fontMetrics().height()),
								Qt::AlignLeft | Qt::AlignVCenter,
								QString::number(temp + start));
		}
		emit ImageUpdated();
	}
}
