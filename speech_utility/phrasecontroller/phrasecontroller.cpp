#include "phrasecontroller.h"
#include "../waveviewerwidget/waveviewerwidget.h"
#include "phrasesList/phraseslist.h"
#include <QPainter>
#include <iostream>
#include <assert.h>
#include <math.h>
#include "phraseslist/phraseslistdata/phraseslistdata.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QDragEnterEvent>
#include <QUrl>
#include "phrase/phrase.h"

PhraseController::PhraseController(QWidget * parent, ButtonsController * _buttonController):
	QScrollArea(parent),
	buttonController(_buttonController),
	curList(NULL),
	max(0),
	samplerate(8000),
	vLayout(NULL),
	vSpacer(NULL),
	phrasesListMenu(NULL),
	preferredColorsIndex(0),
	searchList(NULL),
	savePhraseTextWE(false)
{
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnCustomContextMenuRequested(const QPoint &)));
	setAcceptDrops(true);
	preferredColors.append(Qt::white);
	preferredColors.append(Qt::red);
	preferredColors.append(Qt::green);
	preferredColors.append(Qt::cyan);
	preferredColors.append(Qt::magenta);
	preferredColors.append(Qt::yellow);
	preferredColors.append(Qt::darkRed);
	preferredColors.append(Qt::darkGreen);
	preferredColors.append(Qt::darkGreen);
	preferredColors.append(Qt::darkBlue);
	preferredColors.append(Qt::darkCyan);
	preferredColors.append(Qt::darkMagenta);
	preferredColors.append(Qt::darkYellow);
}

PhraseController::~PhraseController()
{
	//! Удалим
	foreach(PhrasesList * list, lists)
	{
		delete list;
	}
}

PhrasesList * PhraseController::AddList()
{
	PhrasesList * list = new PhrasesList(*this);
	lists.append(list);
	SetCurList(list);

	if ( !vLayout )
	{
		vLayout = new QVBoxLayout(widget());
		vLayout->setContentsMargins(2,0,0,0);
	}
	if ( !vSpacer )
		vSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

	vLayout->addWidget(list);
	vLayout->removeItem(vSpacer);
	vLayout->addItem(vSpacer);
	list->show();
	list->GetData().SetColor(preferredColors[preferredColorsIndex]);
	preferredColorsIndex++;
	preferredColorsIndex = preferredColorsIndex % preferredColors.size();
	return list;
}

bool PhraseController::RemoveList(PhrasesList * list)
{
	int index = lists.indexOf(list);
	if ( index != -1 )
	{
		delete lists.takeAt(index);

		if ( curList == list )
		{
			curList = NULL;
			searchList = NULL;

			if ( lists.size() )
				SetCurList(lists[index < lists.size() ? index :  lists.size() -1 ]);
			else 
			{
				emit CurrentListChanged();
				emit PhraseDataChanged();
			}
		}
		return true;
	}
	return false;
}

PhrasesListData * PhraseController::GetCurPhraseListData() const
{
	if ( curList )
		return &curList->GetData();
	return NULL;
}


Phrase * PhraseController::GetCurPhrase() const 
{ 
	return curList ? curList->GetData().GetCurPhrase() : NULL; 
}

bool PhraseController::SetCurPhrase(Phrase * phrase)
{
	if ( phrase )
	{
		SetCurList(phrase->GetListData().GetList());
		return curList->GetData().SetCurPhrase(phrase);
	}
	return false;
}


bool PhraseController::SetCurList(PhrasesList * list)
{
	if ( list != curList )
	{
		int index = lists.indexOf(list);
		if ( index != -1 )
		{
			if (curList)
				curList->GetData().SetSelected(false);
			curList = list;
			list->GetData().SetSelected(true);
			emit CurrentListChanged();
			emit PhraseDataChanged();
			return true;
		}
		else if ( !list )
		{
			curList = NULL;
			emit CurrentListChanged();
			emit PhraseDataChanged();
		}
	}
	return false;
}

void PhraseController::SetMax(long _max)
{
	if ( max != _max )
	{
		max = _max;

		foreach(PhrasesList * list, lists)
		{
			list->GetData().UpdateMax();
		}
	}
}

void PhraseController::OnCustomContextMenuRequested(const QPoint & pos)
{
	if ( phrasesListMenu )
		phrasesListMenu->exec(mapToGlobal(pos));
}

void PhraseController::MoveUp()
{
	if ( curList )
	{
		int index = vLayout->indexOf(curList);
		if ( index > 0 )
		{
			vLayout->removeWidget(curList);
			vLayout->insertWidget(index-1, curList);
		}
	}
}

void PhraseController::MoveDown()
{
	if ( curList )
	{
		int index = vLayout->indexOf(curList);
		if ( index < vLayout->count()-2 )
		{
			vLayout->removeWidget(curList);
			vLayout->insertWidget(index+1, curList);
		}
	}
}


void PhraseController::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void PhraseController::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void PhraseController::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void PhraseController::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls()) 
	{
		QList<QUrl> urlList = mimeData->urls();
		QString text;
		for (int i = 0; i < urlList.size() && i < 32; ++i) 
		{
			emit DropFile(urlList[i].toLocalFile());
		}
		text = text;
    }
	event->acceptProposedAction();
}


void PhraseController::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		window = _window;
		foreach(PhrasesList * list, lists)
			list->update();
	}
}

void PhraseController::SetSamplerate(int _samplerate)
{
	if ( samplerate != _samplerate )
	{
		samplerate = _samplerate;

		foreach(PhrasesList * list, lists)
		{
			list->update();
		}
	}
}

qreal PhraseController::ToWindowUnit(long sample) const
{
	return 1./(qreal)samplerate * sample;
}



long PhraseController::ToSample(qreal time, bool forSelect) const
{
	qreal r = time/(1./(qreal)samplerate);
	qreal intPart;
	if ( forSelect )
		return r >= 0 ? (modf(r, &intPart) != 0 ? intPart + 1 : intPart) : 0;
	else 
		return r >= 0 ? (modf(r, &intPart) < 0.5 ? floor(r) : ceil(r)) : 0;
}

void PhraseController::RemoveFragment(long startSample, long endSample)
{
	foreach(PhrasesList * list, lists)
	{
		list->GetData().RemoveFragment(startSample, endSample);
	}
}

void PhraseController::InsertFragment(long startSample, long count)
{
	foreach(PhrasesList * list, lists)
	{
		list->GetData().InsertFragment(startSample, count);
	}
}

Phrase * PhraseController::SearchPhrase(const QString & name, bool useCaseSensitive, bool useWholeWord, SearchType type)
{
	if ( curList )
	{
		//if ( type == SearchInCurrentList )
		{
			bool loop;
			return curList->GetData().SearchPhrase(name, useCaseSensitive, useWholeWord, loop);
		}
		//else if ( type == SearchInAll )
		//{
		//	int index = lists.indexOf(curList);
		//	int count = 0;

		//	while ( count < lists.size() )
		//	{
		//		bool loop;
		//		Phrase * phrase = lists[index]->GetData().SearchPhrase(name, useCaseSensitive, useWholeWord, loop);
		//		if ( !loop && phrase )
		//			return phrase;
		//		index = (index + 1) % lists.size();
		//		count++;
		//	}
		//}
	}
	return NULL;
}

void PhraseController::RemoveAll()
{
	QList<PhrasesList * > data = lists;
	foreach(PhrasesList * list, data)
	{
		RemoveList(list);
	}
	SetCurList(NULL);
	preferredColorsIndex = 0;
}

void PhraseController::SetMarkingFont(const QFont & _font)
{
	if ( font != _font )
	{
		font = _font;
		update();
	}
}