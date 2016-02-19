#include "phrase.h"
#include <iostream>
#include <assert.h>
#include <math.h>
#include "../phrasesList/phraseslistdata/phraseslistdata.h"
#include "../../waveviewerwidget/buttonscontroller.h"

Phrase::Phrase(PhrasesListData & _list, long _start, long _end, const QString & _name):
	list(_list),
	start(_start), 
	end(_end), 
	name(_name),
	startButton(NULL),
	endButton(NULL)
{
	if ( end < start )
		end  = start;
	//assert(end >= start);
	//assert(!name.isEmpty());
}

Phrase::Phrase(const Phrase & phrase):
	list(phrase.list),
	name(phrase.name),
	start(phrase.start),
	end(phrase.end),
	startButton(NULL),
	endButton(NULL)
{}

Phrase::~Phrase()
{
	Button * b1 = startButton;
	Button * b2 = endButton;
	startButton = NULL;
	endButton = NULL;
	delete b1;
	delete b2;
}

Phrase & Phrase::operator = (const Phrase & phrase)
{
	name = phrase.name;
	start = phrase.start;
	end = phrase.end;
	return *this;
}


bool Phrase::operator == (const Phrase & phrase)
{
	return start == phrase.start && end == phrase.end;
}

void Phrase::SetStart(long v)
{
	QPair<long, long> border = GetBorder();

	if ( v < border.first )
		v = border.first;

	if ( v > end )
		v = end;

	if ( v != start )
	{
		start = v;
		list.DataChanged();
	}
}

void Phrase::MoveStart(long v)
{
	QPair<long, long> border = GetBorder();

	if ( v < border.first )
		v = border.first;


	long len = GetLen();

	if ( v + len > border.second )
		v = border.second - len;

	if ( v != start )
	{
		start = v;
		end = start + len;
		list.DataChanged();
	}
}


void Phrase::SetEnd(long v)
{
	QPair<long, long> border = GetBorder();

	if ( v > border.second )
		v = border.second;

	if ( v < start )
		v = start;

	if ( v != end )
	{
		end = v;
		list.DataChanged();
	}
}

void Phrase::SetLen(long v)
{
	if ( v > 0 )
		SetEnd(start + v);
}

void Phrase::SetName(const QString & _name)
{
	if ( _name != name )
	{
		name = _name;
		list.UpdateString();
		list.DataChanged();
	}
}

QPair<long, long> Phrase::GetBorder()
{
	const QList<Phrase * > & phrases = list.GetPhrases();

	if ( !list.IsIntersected() )
	{
		int index = phrases.indexOf(this);
		if ( index == 0 )
		{
			return QPair<long, long>(0, phrases.size() == 1 ? 
				list.GetMax() : phrases[1]->GetStart());
		}
		else if ( index == phrases.size()-1 )
		{
			return QPair<long, long>(phrases[index-1]->GetEnd(), list.GetMax());
		}

		return QPair<long, long>(phrases[index-1]->GetEnd(), phrases[index+1]->GetStart());
	}
	else
		return QPair<long, long>(0, list.GetMax());
}

void Phrase::SetButton(Button * button, bool start)
{
	if ( start )
	{
		delete startButton;
		startButton = button;
	}
	else 
	{
		delete endButton;
		endButton = button;
	}
}

const QColor & Phrase::GetColor() const
{
	return list.GetColor();
}