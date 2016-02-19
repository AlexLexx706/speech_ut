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
	//! Конструктор.
	Button(ButtonsController * controller = NULL);

	//! Деструктор.
	virtual ~Button();

	//! Положение в координатах виджета.
	virtual QRect GetRect() const = 0;

	//! Переместить кнопку.
	virtual void Move(QMouseEvent * event){};

	//! Выделить.
	virtual void MousePress(QMouseEvent * event){};

	//! Выделить.
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

//! Контролирует выбор кнопок.
class SPEECH_UTILITY_EXPORT ButtonsController
{
public:
	//! Конструктор.
	ButtonsController(WaveViewerWidget & widget);
	
	//! Деструктор.
	~ButtonsController();

	//! Добавить.
	bool AddButton(Button * button);

	//! Удалить.
	bool RemoveButton(Button * button);

	//! Нажали мышкой на кнопку, pos в координатах окна.
	Button * SelectAndPressButton(QMouseEvent * event);

	//! Отпустили мышку с кнопки.
	void ReleaseButton(QMouseEvent * event);

	//! Получить кнопку.
	Button * GetButton(const QPoint & pos);

	//! Текущая кнопка.
	Button * GetCurrentButton();

	//! Получить список кнопок.
	const QList< Button * > & GetData() const;

	WaveViewerWidget & GetWidget() const;

private:
	
	WaveViewerWidget & widget;
	
	QList< Button * > buttons;

	Button * currentButton;
};

#endif // PHRASE_CONTROLLER_H
