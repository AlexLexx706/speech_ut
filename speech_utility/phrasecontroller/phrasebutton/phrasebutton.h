#ifndef PHRASE_BUTTON_H
#define PHRASE_BUTTON_H

#include "../../waveviewerwidget/buttonscontroller.h"
#include <QVector>
#include <QPair>
#include <QRectF>


class Phrase;

//! Контрол управления границами фразы.
class PhraseButton:public Button
{
public:
	//! Конструктор.
	PhraseButton(ButtonsController * controller, Phrase & _phrase, bool _start);

	~PhraseButton();

	//! Положение в координатах виджета.
	virtual QRect GetRect() const;

	//! Переместить кнопку.
	virtual void Move(QMouseEvent * event);

	//! Выделить.
	virtual void MousePress(QMouseEvent * event);

	virtual QColor GetColor()const;

	//! Получить фразу.
	Phrase * GetPhrase() const;

private:

	//!Ссылка на фразу.
	Phrase & phrase;

	//! Управляет началом фразы. 
	bool start;
	
	//! Размер.
	int width;

	//! Смещение.
	QPoint offset;
};

#endif // PHRASE_BUTTON_H
