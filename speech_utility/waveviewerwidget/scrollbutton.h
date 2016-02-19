#ifndef SCROLL_BUTTON_H
#define SCROLL_BUTTON_H

#include "buttonscontroller.h"
class WaveViewerWidget;


class ScrollButton:public Button
{
public:
	//! �����������.
	ScrollButton(ButtonsController * controller, WaveViewerWidget & widget);

	//! ��������� � ����������� �������.
	virtual QRect GetRect() const;

	//! ����������� ������.
	virtual void Move(QMouseEvent * event);

	//! ��������.
	virtual void MousePress(QMouseEvent * event);

	//! �������� ������������ �������������.
	QRect GetDisplayedRect() const;

	//! ������� ���������� �������������.
	QRect GetSelectedRect() const;

private:
	enum Action
	{
		NoAction,
		MoveLeftBorder,
		MoveRightBorder,
		MoveButton
	};

	WaveViewerWidget & widget;

	//! ������ �������.
	int borderWidth;

	//! ��������.
	QPoint offset;

	//! ��������.
	Action action;
};

#endif // SCROLL_BUTTON_H
