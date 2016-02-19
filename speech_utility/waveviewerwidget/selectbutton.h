#ifndef SELECT_BUTTON_H
#define SELECT_BUTTON_H

#include "buttonscontroller.h"
class WaveViewerWidget;


class SelectButton:public Button
{
public:
	//! �����������.
	SelectButton(ButtonsController * controller, 
				WaveViewerWidget & widget, 
				bool start);

	//! ��������� � ����������� �������.
	virtual QRect GetRect() const;

	//! ����������� ������.
	virtual void Move(QMouseEvent * event);

	//! ��������.
	virtual void MousePress(QMouseEvent * event);

	//! ���������� ������.
	void SetNeighbor(SelectButton * _neighbor){ neighbor = _neighbor; }

	//! ��������� �������.
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
