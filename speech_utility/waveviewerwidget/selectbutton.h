#ifndef SELECT_BUTTON_H
#define SELECT_BUTTON_H

#include "buttonscontroller.h"
class WaveViewerWidget;


class SelectButton:public Button
{
public:
	//! Конструктор.
	SelectButton(ButtonsController * controller, 
				WaveViewerWidget & widget, 
				bool start);

	//! Положение в координатах виджета.
	virtual QRect GetRect() const;

	//! Переместить кнопку.
	virtual void Move(QMouseEvent * event);

	//! Выделить.
	virtual void MousePress(QMouseEvent * event);

	//! Установить соседа.
	void SetNeighbor(SelectButton * _neighbor){ neighbor = _neighbor; }

	//! Управляет стартом.
	bool IsStart() const {return start;}

private:
	WaveViewerWidget & widget;
	
	bool start;

	QSize size;

	QPoint offset;

	//!
	SelectButton * neighbor;
};

#endif // SELECT_BUTTON_H
