#include "selectbutton.h"
#include "waveviewerwidget.h"
#include <QPainter>
#include <QMouseEvent>

SelectButton::SelectButton(ButtonsController * controller, WaveViewerWidget & widget, bool _start):
	Button(controller), 
	widget(widget),
	start(_start),
	size(8,8)
{
}

QRect SelectButton::GetRect() const
{
	const QRect & controlRect = widget.GetControlsRect();
	qreal offset = 0.5;

	const QRectF & window = widget.GetWindow();

	if ( start )
	{
		int x = controlRect.left() + controlRect.width() * (widget.GetSelectedSamples().first - offset - window.left())/ window.width();
		return QRect(x, controlRect.top(), size.width(), size.height());
	}
	else 
	{
		int x = controlRect.left() + controlRect.width() * (widget.GetSelectedSamples().second - offset - window.left())/ window.width();
		return QRect(x - size.width(), controlRect.top(),
			size.width(), size.height());
	}
}

void SelectButton::Move(QMouseEvent * event)
{
	QPoint pos = event->pos() + offset;
	const QRect & rect = widget.GetControlsRect();

	if ( start )
	{
		if ( pos.x() > rect.right() )
			pos.rx() = rect.right();

		if ( pos.x() < rect.left() )
			pos.rx() = rect.left();

		long sample = widget.ToSelectedSample(widget.Transform(pos).x());
		long rBorder = widget.GetSelectedSamples().second;
		
		//! Поменялись местами.
		if ( sample > rBorder )
		{
			neighbor->start = true;
			start = false;
			widget.SelectSamples(rBorder, sample, false);
		}
		else 
			widget.SelectSamples(sample, rBorder, false);
	}
	else
	{
		if ( pos.x() > rect.right() - size.width() )
			pos.rx() = rect.right() - size.width();

		if ( pos.x() < rect.left() )
			pos.rx() = rect.left();

		long sample = widget.ToSelectedSample(widget.Transform(QPoint(pos.x() + size.width(), pos.y())).x());
		long lBorder = widget.GetSelectedSamples().first;

		if ( sample < lBorder )
		{
			neighbor->start = false;
			start = true;
			widget.SelectSamples(sample, lBorder, false);			
		}
		else 
			widget.SelectSamples(lBorder, sample, false);
	}
}


void SelectButton::MousePress(QMouseEvent * event)
{
	offset = GetRect().topLeft() - event->pos();	
}