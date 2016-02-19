#include "frame.h"

///////////////////////////////////////////////////////////////
Frame::Frame():
	isReady(true)
{}

void Frame::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		window =_window;
		Update();
		emit WindowChanged(window);
	}
}

void Frame::SetSize(const QSize & _size)
{
	if ( pixmap.size() != _size )
	{
		pixmap = QPixmap(_size);
		Update();
	}
}
