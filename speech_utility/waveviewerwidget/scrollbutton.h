#ifndef SCROLL_BUTTON_H
#define SCROLL_BUTTON_H

#include "buttonscontroller.h"
class WaveViewerWidget;


class ScrollButton:public Button
{
public:
	//! Конструктор.
	ScrollButton(ButtonsController * controller, WaveViewerWidget & widget);

	//! Положение в координатах виджета.
	virtual QRect GetRect() const;

	//! Переместить кнопку.
	virtual void Move(QMouseEvent * event);

	//! Выделить.
	virtual void MousePress(QMouseEvent * event);

	//! Получить отображаемый прямоугольник.
	QRect GetDisplayedRect() const;

	//! Поучить выделенный прямоугольник.
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

	//! Ширина границы.
	int borderWidth;

	//! Смещение.
	QPoint offset;

	//! Действие.
	Action action;
};

#endif // SCROLL_BUTTON_H
