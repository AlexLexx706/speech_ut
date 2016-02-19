#include "hrullerframe.h"
#include <QPainter>
#include <math.h>
#include <assert.h>
#include <QTime>
#include "../../../mainwindow.h"

////////////////////////////////////////////////////////////////
void HRullerFrame::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		if ( window.width() != _window.width() || window.left() != _window.left() )
		{
			window = _window;
			Update();
		}
		else 
			window = _window;
	}
}
void HRullerFrame::SetSampleRate(int rate)
{
	if ( rate != sampleRate )
	{
		sampleRate = rate;
		Update();
	}
}



QString ConvertToString2(qreal t)
{
	return MainWindow::FromSecToStrign(t);
}


void HRullerFrame::Update()
{
	if ( !pixmap.isNull() && window.width() )
	{
		QPainter painter(&pixmap);
		pixmap.fill(Qt::gray);

		//Разметка.
		qreal dx = pixmap.width()/window.width();

		long wCount = pixmap.width()/cellWidth;
		qreal wStep = window.width()/wCount;
		if ( wStep  < 1. )wStep = 1.;

		qreal start = ((long)(window.left()/wStep)) * wStep;
		qreal wStart = start - window.left();
		wCount = window.width()/wStep + 1;

		painter.setPen(Qt::white);
		painter.drawLine(0, 0, pixmap.width(), 0);
		painter.setFont(QFont("Arial", 7));

		marks.resize(wCount);

		//семплы
		if ( showSamples )
		{
			for (int i = 0; i < wCount; i++ )
			{
				qreal temp = wStep * i;
				qreal x = (wStart + temp) * dx;
				marks[i] = x;
				painter.drawLine(x, 0, x, 2);
				painter.drawText(x, painter.fontMetrics().height(), QString("%1").arg((long)(start + temp)));
			}
		}
		else 
		{
			qreal k = 1./sampleRate;

			for (int i = 0; i < wCount; i++ )
			{
				qreal temp = wStep * i;
				qreal x = (wStart + temp) * dx;
				marks[i] = x;
				painter.drawLine(x, 0, x, 2);
				painter.drawText(x, painter.fontMetrics().height(),
									ConvertToString2((start + temp)*k));
			}
		}
		emit ImageUpdated();
	}
}
