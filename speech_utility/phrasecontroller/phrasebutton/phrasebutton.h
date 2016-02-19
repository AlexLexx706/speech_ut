#ifndef PHRASE_BUTTON_H
#define PHRASE_BUTTON_H

#include "../../waveviewerwidget/buttonscontroller.h"
#include <QVector>
#include <QPair>
#include <QRectF>


class Phrase;

//! ������� ���������� ��������� �����.
class PhraseButton:public Button
{
public:
	//! �����������.
	PhraseButton(ButtonsController * controller, Phrase & _phrase, bool _start);

	~PhraseButton();

	//! ��������� � ����������� �������.
	virtual QRect GetRect() const;

	//! ����������� ������.
	virtual void Move(QMouseEvent * event);

	//! ��������.
	virtual void MousePress(QMouseEvent * event);

	virtual QColor GetColor()const;

	//! �������� �����.
	Phrase * GetPhrase() const;

private:

	//!������ �� �����.
	Phrase & phrase;

	//! ��������� ������� �����. 
	bool start;
	
	//! ������.
	int width;

	//! ��������.
	QPoint offset;
};

#endif // PHRASE_BUTTON_H
