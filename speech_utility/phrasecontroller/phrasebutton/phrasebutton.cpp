#include "phrasebutton.h"
#include "../phrase/phrase.h"
#include "../../waveviewerwidget/waveviewerwidget.h"
#include "../phraseslist/phraseslistdata/phraseslistdata.h"
#include <QPainter>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <QMouseEvent>


PhraseButton::PhraseButton(ButtonsController * controller, Phrase & _phrase, bool _start):
	Button(controller),
	phrase(_phrase),
	start(_start),
	width(10)
{
	phrase.SetButton(this, start);
}

PhraseButton::~PhraseButton()
{
	//phrase.SetButton(NULL, start);
}


QRect PhraseButton::GetRect() const
{
	if ( visible )
	{
		WaveViewerWidget & widget = controller->GetWidget();

		const QRect & plotRect = widget.GetPlotRect();
		const QRectF & window = widget.GetWindow();
		int x = plotRect.left() + plotRect.width() * ((start ? phrase.GetStart() : phrase.GetEnd()) - window.left())/ window.width();
		return QRect(x - (width >> 1), plotRect.top(), width, plotRect.height());
	}
	return QRect();
}

void PhraseButton::Move(QMouseEvent * event)
{
	QPoint pos = offset + event->pos();
	pos.rx() = pos.x() + (width >> 1);

	WaveViewerWidget & widget = controller->GetWidget();
	if ( start )
		phrase.SetStart(widget.ToViewSample(widget.Transform(pos).x()));
	else 
		phrase.SetEnd(widget.ToViewSample(widget.Transform(QPoint(pos.x() + width, pos.y())).x()));
}

void PhraseButton::MousePress(QMouseEvent * event)
{
	offset = GetRect().topLeft() - event->pos();
	phrase.GetListData().SetCurPhrase(&phrase);
}

QColor PhraseButton::GetColor()const
{
	return phrase.GetColor();
}

Phrase * PhraseButton::GetPhrase() const
{
	return &phrase;
}
