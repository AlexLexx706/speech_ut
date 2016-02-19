#include "scrollbutton.h"
#include "waveviewerwidget.h"
#include <QPainter>
#include <iostream>
#include <QMouseEvent>

ScrollButton::ScrollButton(ButtonsController * controller, WaveViewerWidget & widget):
	Button(controller),
	widget(widget),
	borderWidth(3),
	action(NoAction)
{
}

QRect ScrollButton::GetRect() const
{
	const QRect & controlRect = widget.GetScrollRect();
	const QRectF & maxWindow = widget.GetMaxWindow();
	const QRectF & window = widget.GetWindow();

	long x1 = (window.left() - maxWindow.left())/maxWindow.width() * controlRect.width() + controlRect.left() - borderWidth;
	long x2 = (window.right() - maxWindow.left())/maxWindow.width() * controlRect.width() + controlRect.left() + borderWidth;

	return QRect(x1, controlRect.top(), x2 - x1, controlRect.height());
}

void ScrollButton::Move(QMouseEvent * event)
{
	QPoint pos = event->pos() + offset;
	QRect controlRect = widget.GetScrollRect();
	QRectF maxWindow = widget.GetMaxWindow();
	QRectF window = widget.GetWindow();

	qreal x = (pos.x()  - controlRect.left()) / (qreal)controlRect.width() * maxWindow.width();

	if ( action == MoveLeftBorder )
	{
		x = x > window.right() - widget.GetMinWindowSize().width() ? window.right() - widget.GetMinWindowSize().width() : x;
		widget.SetWindow(QRectF(x, window.top(), window.right() - x, window.height()));
	}
	else if ( action == MoveRightBorder )
	{
		x = x < window.left() ? window.left() : x;
		QRectF r(window.left(), window.top(), x - window.left(), window.height());
		widget.SetWindow(r);
	}
	else if ( action == MoveButton)
	{
		widget.SetWindow(QRectF(x, window.top(), window.width(), window.height()));
	}
}

void ScrollButton::MousePress(QMouseEvent * event)
{
	QPoint pos = event->pos();
	QRect rect = GetRect();
	QRect displayRect = GetDisplayedRect();
	int dw = displayRect.width()/2;
	int width = borderWidth <= dw ? borderWidth : dw;

	action = NoAction;

	//! перемещение.
	if ( !(event->modifiers() & Qt::ControlModifier) )
	{
		action = MoveButton;
		offset = GetRect().topLeft() - pos;
	}//! Левая граница.
	else if ( pos.x() >= rect.left() && pos.x() <= displayRect.left() + width )
	{
		action = MoveLeftBorder;
		offset = GetRect().topLeft() - pos;
	}
	//! Правая граница
	else if ( pos.x() >= displayRect.right() - width && pos.x() <= rect.right() )
	{
		action = MoveRightBorder;
		offset = GetRect().topRight() - pos;
	}
}

QRect ScrollButton::GetDisplayedRect() const
{
	QRect rect = GetRect();
	return QRect(rect.left() + borderWidth, rect.top(), rect.width() - borderWidth*2, rect.height());
}

QRect ScrollButton::GetSelectedRect() const
{
	QRect controlRect = widget.GetScrollRect();
	QRectF maxWindow = widget.GetMaxWindow();
	QPair<long, long> samples = widget.GetSelectedSamples();

	long x1 = widget.ToSec(samples.first)/maxWindow.width() * controlRect.width() + controlRect.left();
	long x2 = widget.ToSec(samples.second)/maxWindow.width() * controlRect.width() + controlRect.left();

	return QRect(x1, controlRect.top(), x2 - x1, controlRect.height());
}