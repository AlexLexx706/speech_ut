#ifndef BUTTONS_CONTROLLER_H
#define BUTTONS_CONTROLLER_H

#include <QVector>
#include <QPair>
#include <QRectF>
#include <QColor>
#include "../speech_utility_global.h"

class WaveViewerWidget;
class ButtonsController;
class QMouseEvent;

class SPEECH_UTILITY_EXPORT Button
{
public:
	//! �����������.
	Button(ButtonsController * controller = NULL);

	//! ����������.
	virtual ~Button();

	//! ��������� � ����������� �������.
	virtual QRect GetRect() const = 0;

	//! ����������� ������.
	virtual void Move(QMouseEvent * event){};

	//! ��������.
	virtual void MousePress(QMouseEvent * event){};

	//! ��������.
	virtual void MouseRelease(QMouseEvent * event){};
	
	virtual void SetColor(const QColor & color){};

	virtual QColor GetColor()const { return QColor(); }

	void SetVisible(bool show){ visible = show;}

	bool IsVisible() const {return visible; }

protected:
	//! 
	ButtonsController * controller;

	bool visible;
};

//! ������������ ����� ������.
class SPEECH_UTILITY_EXPORT ButtonsController
{
public:
	//! �����������.
	ButtonsController(WaveViewerWidget & widget);
	
	//! ����������.
	~ButtonsController();

	//! ��������.
	bool AddButton(Button * button);

	//! �������.
	bool RemoveButton(Button * button);

	//! ������ ������ �� ������, pos � ����������� ����.
	Button * SelectAndPressButton(QMouseEvent * event);

	//! ��������� ����� � ������.
	void ReleaseButton(QMouseEvent * event);

	//! �������� ������.
	Button * GetButton(const QPoint & pos);

	//! ������� ������.
	Button * GetCurrentButton();

	//! �������� ������ ������.
	const QList< Button * > & GetData() const;

	WaveViewerWidget & GetWidget() const;

private:
	
	WaveViewerWidget & widget;
	
	QList< Button * > buttons;

	Button * currentButton;
};

#endif // PHRASE_CONTROLLER_H
