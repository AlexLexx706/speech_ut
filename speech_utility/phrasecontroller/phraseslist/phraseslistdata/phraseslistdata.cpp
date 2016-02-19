#include "phraseslistdata.h"
#include "../phraseslist.h"
#include "../../../waveviewerwidget/buttonscontroller.h"
#include "../../phrasecontroller.h"
#include "../../phrasebutton/phrasebutton.h"
#include "../../../waveviewerwidget/waveviewerwidget.h"
#include "../../phrase/phrase.h"
#include <QPainter>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <QMouseEvent>
#include <QLineEdit>
#include "extlineedit/extlineedit.h"
#include <iostream>


bool HasProjection(long sA, long eA, long sB, long eB, long & offsetAB)
{
	long aL = (eA - sA) >> 1;
	long bL = (eB - sB) >> 1;
	offsetAB = (sA + aL) - (sB + bL);
	return abs(offsetAB) < aL + bL;
}


PhrasesListData::PhrasesListData(QWidget * parent, QList<Phrase *> & data):
	QFrame(parent),
	controller(NULL),
	curPhrase(NULL), 
	intersected(false),
	color(new QColor(Qt::red)),
	selected(false),
	selectedWidth(4),
	action(NoAction),
	lineEdit(NULL),
	blockEdit(false),
	startSearch(NULL),
	is_edit_disable(false)
{
	list = qobject_cast<PhrasesList *>(parent);
	assert(list);
	controller = &list->GetController();
	SetPhrases(data);
	connect(this, SIGNAL(CurrentPhraseChanged()), controller, SIGNAL(CurrentPhraseChanged()));
	connect(this, SIGNAL(PhraseDataChanged()), controller, SIGNAL(PhraseDataChanged()));
	connect(this, SIGNAL(PhraseDoubleClick()), controller, SIGNAL(PhraseDoubleClick()));
	connect(this, SIGNAL(ScaleHRuller(qreal, int)), controller, SIGNAL(ScaleHRuller(qreal, int)));
	connect(this, SIGNAL(ListDataChanged()), controller, SIGNAL(ListDataChanged()));
	setMouseTracking(true);
}

PhrasesListData::~PhrasesListData()
{
	//! Удалим кнопки старых фраз.
	foreach(Phrase * phrase, phrases)
	{
		delete phrase;	
	}
	delete color;
}


void PhrasesListData::SetIntersected(bool v)
{
	if ( v != intersected )
	{
		intersected = v;
		
		//Перестроим метки.
		if ( !intersected )
		{
			QList<Phrase *> sortedList;
			QList<Phrase *> _phrases(phrases);

			foreach(Phrase * phrase, _phrases)
			{
				if ( sortedList.size() )
				{
					for (int i = 0; i < sortedList.size(); i++ )
					{
						if ( phrase->GetStart() < sortedList[i]->GetStart() )
						{
							sortedList.insert(i, phrase);
							break;
						}
						else if ( phrase->GetStart() <= sortedList[i]->GetStart() )
						{
							RemovePhrase(phrase, NoMoveBorder);
							break;
						}
						else if ( i == sortedList.size()-1 )
						{
							sortedList.append(phrase);
							break;
						}
					}
				}
				else 
				{
					sortedList.append(phrase);
				}
			}
			phrases = sortedList;

			//! Обновим длины.
			foreach(Phrase * phrase, phrases)
			{
				phrase->SetEnd(phrase->GetEnd());
			}
		}
		DataChanged();
	}
}

void PhrasesListData::SetPhrases(const QList<Phrase *> & data)
{
	//SetIntersected(true);

	//! Удалим кнопки старых фраз.
	foreach(Phrase * phrase, phrases)
		delete phrase;

	phrases = data;

	//!Обрежим по максимуму.
	UpdateMax();

	//! Добавим кнопки.
	foreach(Phrase * phrase, phrases)
	{
		PhraseButton * startButton = new PhraseButton(controller->GetButtonController(), *phrase,true);
		PhraseButton * endButton = new PhraseButton(controller->GetButtonController(), *phrase,false);
	}
	UpdateString();
	if ( curPhrase != NULL )
	{
		if ( curPhrase == startSearch )
			startSearch = NULL;

		curPhrase = NULL;
		emit CurrentPhraseChanged();
	}
	update();
}


long PhrasesListData::GetMax() const
{
	return controller->GetMax();
}

bool PhrasesListData::CutPhrases(long pos, bool copyText)
{
	if ( pos > 0 )
	{
		for ( int i = 0; i < phrases.size(); i++ )
		{
			if ( pos >  phrases[i]->GetStart() && pos < phrases[i]->GetEnd() )
			{
				Phrase * p1 = new Phrase(*this, phrases[i]->GetStart(), pos, phrases[i]->GetName());
				new PhraseButton(controller->GetButtonController(), *p1,true);
				new PhraseButton(controller->GetButtonController(), *p1,false);

				Phrase * p2 = new Phrase(*this, pos, phrases[i]->GetEnd(), copyText ? phrases[i]->GetName() : "");
				new PhraseButton(controller->GetButtonController(), *p2,true);
				new PhraseButton(controller->GetButtonController(), *p2,false);

				phrases.insert(i+1, p1);
				phrases.insert(i+2, p2);

				Phrase * temp = phrases.takeAt(i);
				delete temp;
				UpdateString();

				if ( temp == curPhrase )
				{
					if ( temp == startSearch )
						startSearch = NULL;

					curPhrase = NULL;
					emit CurrentPhraseChanged();
				}
				update();
				return true;
			}
		}		
	}
	return false;
}

bool PhrasesListData::InsertPhrase(Phrase * phrase, bool copyText)
{
	if ( phrase && phrase->GetStart() >= 0 && phrase->GetEnd() <= GetMax() )
	{
		CutPhrases(phrase->GetStart(),copyText);
		CutPhrases(phrase->GetEnd(),true);

		int startIndex = -1;
		int insertIndex = -1;
		int count = 0;

		//Делим на части.
		Phrase * tempPhrase;
		long offset;

		for ( int i = 0; i < phrases.size(); i++ )
		{
			tempPhrase = phrases[i];
			if ( tempPhrase->GetStart() >= phrase->GetStart() && 
				tempPhrase->GetEnd() <= phrase->GetEnd() )
			{
				if ( startIndex == -1 )
					startIndex = i;
				count++;
			}
			else if (phrase->GetStart() >= tempPhrase->GetEnd())
				insertIndex  = i;
		}

		//Удалим лишнее.
		if ( startIndex != -1 )
		{
			for ( int i = 0; i < count; i++ )
				delete phrases.takeAt(startIndex);

			phrases.insert(startIndex, phrase);
		}
		//Добавим
		else
		{
			insertIndex++;
			phrases.insert(insertIndex, phrase);
		}

		curPhrase = phrase;

		//Кнопки.
		PhraseButton * startButton = new PhraseButton(controller->GetButtonController(), *phrase,true);
		PhraseButton * endButton = new PhraseButton(controller->GetButtonController(), *phrase,false);
		UpdateString();

		emit CurrentPhraseChanged();
		update();
		return true;
	}

	delete phrase;
	return false;
}

bool PhrasesListData::RemovePhrase(Phrase * phrase, RemoveAction action)
{
	int index = phrases.indexOf(phrase);

	if ( index != -1 )
	{
		phrases.takeAt(index);

		if ( action == MoveLeftBorder && index < phrases.size() - 1 )
		{
			phrases[index]->SetStart(phrase->GetStart());
		}
		else if ( action == MoveRightBorder && index > 0 )
		{
			phrases[index-1]->SetEnd(phrase->GetEnd());
		}

		delete phrase;
		UpdateString();

		if (curPhrase == phrase)
		{
			if ( startSearch == phrase )
				startSearch = NULL;

			curPhrase = NULL;

			if ( phrases.size() )
				SetCurPhrase(phrases[(index  < phrases.size() ? index : phrases.size()-1)]);
			else
				emit CurrentPhraseChanged();
		}
		update();
		return true;
	}
	return false;
}

bool PhrasesListData::SetCurPhrase(Phrase * phrase)
{
	if ( phrase != curPhrase )
	{
		int index = phrases.indexOf(phrase);
		if ( index != -1 || 
			index == -1 && phrase == NULL )
		{
			curPhrase = phrase;
			emit CurrentPhraseChanged();
			update();
			return true;
		}
	}
	return false;
}

void PhrasesListData::SetColor(const QColor & _color)
{
	if ( *color != _color )
	{
		*color = _color;
		update();
	}
}



void PhrasesListData::UpdateMax()
{
	//! Обрежим по длинне данных.
	CutPhrases(GetMax(), false);

	//! Удалим фразы разметка которых вне пределах данных.
	QList<Phrase *> delList;
	
	for ( QList<Phrase *>::iterator iter = phrases.begin(); 
		iter != phrases.end(); iter++ )
	{
		if ( (*iter)->GetStart() >= GetMax() )
			delList.append((*iter));
	}

	bool updateCurPhrase = false;

	foreach(Phrase * phrase, delList)
	{
		delete phrases.takeAt(phrases.indexOf(phrase));
		if ( phrase == curPhrase )
			updateCurPhrase = true;
	}
	if ( updateCurPhrase )
	{
		if ( curPhrase == startSearch )
			startSearch = NULL;

		curPhrase = NULL;
		emit CurrentPhraseChanged();
	}

	update();
}

void PhrasesListData::SetSelected(bool v)
{
	if ( selected != v )
	{
		selected = v;
		update();
	}
}

void PhrasesListData::SetButtonsVisible(bool show)
{
	foreach(Phrase * phrase, phrases)
	{
		QPair<Button *,Button *> buttons(phrase->GetButtons());
		buttons.first->SetVisible(show);
		buttons.second->SetVisible(show);
	}
	DataChanged();
}

bool PhrasesListData::RemoveFragment(long startSample, long endSample)
{
	if ( endSample >= startSample )
	{
		CutPhrases(startSample, false);
		CutPhrases(endSample, true);

		QList<Phrase *> deleted;
		QList<Phrase *> moved;

		long offset;
		long len = endSample - startSample + 1;

		foreach(Phrase * phrase, phrases)
		{
			if ( phrase->GetStart() >= startSample && phrase->GetEnd() <= endSample )
				deleted.append(phrase);
			else if ( phrase->GetEnd() > endSample )
				moved.append(phrase);
		}

		bool neadUpdate = false;
		foreach(Phrase * phrase, deleted)
		{
			delete phrases.takeAt(phrases.indexOf(phrase));
			if ( phrase == curPhrase )
				neadUpdate = true;
		}
		if ( neadUpdate )
		{
			if ( curPhrase == startSearch )
				startSearch = NULL;

			curPhrase = NULL;
			emit CurrentPhraseChanged();
		}

		offset = endSample - startSample;
		foreach(Phrase * phrase, moved)
			phrase->MoveStart(phrase->GetStart() - offset);
	}
	return false;
}

bool PhrasesListData::InsertFragment(long startSample, long count)
{
	if ( count )
	{
		Phrase * phrase;
		blockSignals(true);
		for( int i = phrases.size()-1; i >= 0; i--)
		{
			phrase = phrases[i];
			if( phrase->GetStart() > startSample )
			{
				phrase->MoveStart(phrase->GetStart() + count);
			}
			else if ( phrase->GetStart() <= startSample &&
				phrase->GetEnd() >= startSample)
			{
				phrase->SetLen(phrase->GetLen()+count);
			}
		}
		blockSignals(false);
		emit ListDataChanged();
	}
	return false;
}


Phrase * PhrasesListData::SearchPhrase(const QString & name, bool useCaseSensitive, bool useWholeWord, bool & isLast )
{
	if ( phrases.size() )
	{
		int index = 0;
		isLast = false;

		if ( curPhrase )
		{
			if ( (curPhrase->GetName().indexOf(name, 0, useCaseSensitive ?  Qt::CaseSensitive :Qt::CaseInsensitive) == 0) &&
				(!useWholeWord || name.size() == curPhrase->GetName().size()) )
			{
				index = (phrases.indexOf(curPhrase) + 1) % phrases.size();
				
				if ( curPhrase == startSearch )
					isLast = true;
				else if ( !startSearch )
					startSearch = curPhrase;
			}
			else 
			{
				index = (phrases.indexOf(curPhrase) + 1) % phrases.size();
				startSearch = NULL;
			}
		}
		else 
		{
			startSearch = NULL;
		}

		int count = 0;
		while ( count <  phrases.size() )
		{
			Phrase * phrase = phrases[index];
			
			if ( (phrase->GetName().indexOf(name, 0, useCaseSensitive ?  Qt::CaseSensitive :Qt::CaseInsensitive) == 0) &&
				(!useWholeWord || name.size() == phrase->GetName().size()) )
			{
				return phrase;
			}

			index = (index + 1) % phrases.size();
			count++;
		}



		//int index = 0;
		//isLast = false;

		//if ( curPhrase )
		//{
		//	if ( (curPhrase->GetName().indexOf(name, 0, useCaseSensitive ?  Qt::CaseSensitive :Qt::CaseInsensitive) == 0) &&
		//		(!useWholeWord || name.size() == curPhrase->GetName().size()) )
		//	{
		//		index = (phrases.indexOf(curPhrase) + 1) % phrases.size();
		//		if ( !startSearch )
		//			startSearch = curPhrase;
		//	}
		//	else 
		//	{
		//		index = (phrases.indexOf(curPhrase) + 1) % phrases.size();
		//		startSearch = NULL;
		//	}
		//}
		//else 
		//{
		//	startSearch = NULL;
		//}

		//index = FindName(index, name, useCaseSensitive,useWholeWord);
		//if ( index != -1 )
		//{
		//	phrases[index]
		//}
	}

	isLast = true;
	startSearch = NULL;

	return NULL;
}

int PhrasesListData::FindName(int index, const QString & name, bool useCaseSensitive, bool useWholeWord)
{
	int count = 0;
	while ( count <  phrases.size() )
	{
		if ( (phrases[index]->GetName().indexOf(name, 0, useCaseSensitive ?  Qt::CaseSensitive :Qt::CaseInsensitive) == 0) &&
			(!useWholeWord || name.size() == phrases[index]->GetName().size()) )
			return index;
		index = (index + 1) % phrases.size();
		count++;
	}
	return -1;
}




void PhrasesListData::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	painter.setPen(*color);
	painter.setBrush(QColor( 255 - color->red(), 255 - color->green(), 255 - color->blue()));
	painter.drawRect(0,0, width()-1, height()-1);
	const QRectF & window = controller->GetWindow();
	painter.setFont(controller->GetMarkingFont());

	long offset;
	int last;
	bool init = false;

	foreach(Phrase * phrase, phrases)
	{
		int x1 = Translate(phrase->GetStart());
		int x2 = Translate(phrase->GetEnd());
		if ( HasProjection(x1, x2, 0, width(), offset) )
		{
			if ( x2 > x1 )
			{
				if ( (x2 - x1) > 4 )
					painter.drawText(QRect(x1+2, 0, x2-x1-2, height()), phrase->GetName());

				painter.drawLine(x1, 0, x1, height());
				painter.drawLine(x2, 0, x2, height());
			}
			else if ( !init || x2 != last )
			{
				painter.drawLine(x2, 0, x2, height());
				init = true;
			}
			last  = x2;
		}
	}

	if ( curPhrase )
	{
		int x1 = Translate(curPhrase->GetStart());
		int x2 = Translate(curPhrase->GetEnd());
		painter.setBrush(QColor(255,0,0,50));
		painter.drawRect(x1, 0, x2 - x1, height()-1);
	}

	if ( selected )
	{
		painter.setBrush(QColor(0,255,0,50));
		painter.drawRect(0,0, width()-1, height()-1);
	}

	if ( !isEnabled() )
	{
		painter.setBrush(QColor(0,0,255,50));
		painter.drawRect(0,0, width()-1, height()-1);
	}

}

void PhrasesListData::mouseMoveEvent(QMouseEvent * event)
{
	curPos = event->pos();

	if ( action == MoveLeft )
	{
		curPhrase->SetStart(InvTranslate(event->pos().x() + offset.x()));
		setCursor(Qt::SplitHCursor);
	}
	else if ( action == MoveRight )
	{
		curPhrase->SetEnd(InvTranslate(event->pos().x() + offset.x()));
		setCursor(Qt::SplitHCursor);
	}
	else if ( action == MoveAll )
	{
		long len = curPhrase->GetLen();
		curPhrase->MoveStart(InvTranslate(event->pos().x() + offset.x()));
	}
	else if ( action == NoAction )
	{
		
		if ( event->modifiers() == Qt::ControlModifier ) 
		{
			QPoint offset;
			UpdateCursorShape(GetAction(curPhrase, event->pos(), offset));
		}
		else 
		{
			if ( GetPhrase(event->pos()) )
				setCursor(Qt::PointingHandCursor);
			else 
				setCursor(Qt::ArrowCursor);
		}
	}
}

Phrase * PhrasesListData::GetPhrase(const QPoint & pos)
{
	if ( !curPhrase )
	{
		foreach(Phrase *phrase, phrases)
		{
			if ( GetPhraseRect(phrase).contains(pos) )
				return phrase;
		}
	}
	//Поиск по стеку.
	else 
	{
		int index = (phrases.indexOf(curPhrase) + 1) % phrases.size();
		int count = 0;

		while( count < phrases.size()  )
		{
			if ( GetPhraseRect(phrases[index]).contains(pos) )
				return phrases[index];

			index = (index + 1) % phrases.size();
			count++;
		}
	}
	return NULL;
}

void PhrasesListData::DataChanged()
{
	update();
	emit PhraseDataChanged();
}

void PhrasesListData::mousePressEvent(QMouseEvent * event)
{
	if ( event->button() == Qt::LeftButton )
	{
		list->GetController().SetCurList(list);

		//! Просто выделяем фразу.
		if ( event->modifiers() == Qt::NoModifier )
		{
			SetCurPhrase(GetPhrase(event->pos()));
		}
		//! Начнём перемещение.
		else if ( event->modifiers() == Qt::ControlModifier ) 
		{
			if ( !is_edit_disable )
			{
				action = GetAction(curPhrase, event->pos(), offset);
				UpdateCursorShape(action);
			}
		}
		emit clicked();
	}
}

void PhrasesListData::UpdateCursorShape(PhrasesListData::Action act)
{
	switch( act )
	{
		case MoveLeft:
			setCursor(Qt::SplitHCursor);
			break;
		case MoveRight:
			setCursor(Qt::SplitHCursor);
			break;
		case MoveAll:
			setCursor(Qt::SizeAllCursor);
			break;
		default:
			setCursor(Qt::ArrowCursor);
	}
}

PhrasesListData::Action PhrasesListData::GetAction(const Phrase * phrase, const QPoint & pos, QPoint & offset)
{
	if ( phrase )
	{
		QRect rect =  GetPhraseRect(phrase);

		if ( (pos.x() >= (rect.left() - selectedWidth)) && (pos.x() <= (rect.left() + selectedWidth)) )
		{
			offset = rect.topLeft() - pos;
			return MoveLeft;
		}
		else if ( (pos.x() <= (rect.right() + selectedWidth)) && (pos.x() >= (rect.right() - selectedWidth)) )
		{
			offset = rect.topRight() - pos;
			return MoveRight;
		}
		else if ( (pos.x() >= (rect.left() + selectedWidth))  && (pos.x() <= (rect.right() - selectedWidth)) )
		{
			offset = rect.topLeft() - pos;
			return MoveAll;
		}
	}
	return NoAction;
}


void PhrasesListData::mouseReleaseEvent(QMouseEvent * event)
{
	action = NoAction;

	if ( event->modifiers() == Qt::ControlModifier && !is_edit_disable ) 
	{
		QPoint offset;
		UpdateCursorShape(GetAction(curPhrase, event->pos(), offset));
	}
	else 
	{
		if ( GetPhrase(event->pos()) )
			setCursor(Qt::PointingHandCursor);
		else 
			setCursor(Qt::ArrowCursor);
	}
}

void PhrasesListData::mouseDoubleClickEvent(QMouseEvent *)
{
	if ( curPhrase )
	{
		emit PhraseDoubleClick();
		EditPhraseName(curPhrase);
	}
}

void PhrasesListData::EditPhraseName(Phrase * phrase)
{
	if ( phrase && !is_edit_disable )
	{
		QRect rect = GetPhraseRect(phrase);

		if ( !lineEdit )
		{
			lineEdit = new ExtLineEdit(this);
			connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
			connect(lineEdit, SIGNAL(EditInterrupted()), this, SLOT(OnEditingFinished()));
			connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(OnReturnPressed()));
			
			if ( controller->IsSavePhraseTextWE() )
				connect(lineEdit, SIGNAL(textChanged(const QString &) ), this, SLOT(OnTextChanged(const QString &)));
		}

		lineEdit->setText(phrase->GetName());
		lineEdit->move(rect.x(), rect.y());
		lineEdit->resize(200, rect.height());
		lineEdit->raise();
		lineEdit->show();
		lineEdit->setFocus();
		lineEdit->selectAll();
	}
}

QList<Phrase *> PhrasesListData::GetPhrasesByName(const QString & name, bool caseSensitive) const
{
	QList<Phrase *> res;
	
	if ( caseSensitive )
	{
		for ( QList<Phrase * >::const_iterator iter = phrases.constBegin(); 
			iter != phrases.constEnd(); iter++ )
		{
			if ( (*iter)->GetName() == name )
				res.append(*iter);
		}
	}
	else 
	{
		QString str = name.toLower();

		for ( QList<Phrase * >::const_iterator iter = phrases.constBegin(); 
			iter != phrases.constEnd(); iter++ )
		{
			if ( (*iter)->GetName().toLower() == str )
				res.append(*iter);
		}	
	}
	return res;
}

void PhrasesListData::UpdateString()
{
	string = "";
	foreach(Phrase * phrase, phrases)
	{
		string = string + " " + phrase->GetName();
	}
	emit ListDataChanged();
}

void PhrasesListData::keyPressEvent(QKeyEvent * event)
{

	QFrame::keyPressEvent(event);

	if ( curPhrase )
	{
		int index = phrases.indexOf(curPhrase);

		if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
		{
			if ( !is_edit_disable)
			{
				if ( !blockEdit )
				{
					EditPhraseName(curPhrase);
				}
				else
					blockEdit = false;
			}
		}
		else if ( event->key() == Qt::Key_Left )
		{
			//передвижение левой границы (+ граница соседа) влево
			if ( event->modifiers() == Qt::ShiftModifier )
			{
				if ( !is_edit_disable )
				{
					if ( index > 0 )
					{
						Phrase * leftPhrase = phrases[index-1];
						long offset = curPhrase->GetStart() - leftPhrase->GetEnd();
						long pos = InvTranslate(Translate(leftPhrase->GetEnd()) - 1);
						leftPhrase->SetEnd(pos);
						curPhrase->SetStart(leftPhrase->GetEnd() + offset);
					}
					else
						curPhrase->SetStart(InvTranslate(Translate(curPhrase->GetStart()) - 1));
				}
			}
			//передвижение правой границы (+ граница соседа) влево
			else if ( event->modifiers() == Qt::ControlModifier )
			{
				if ( !is_edit_disable)
				{
					if ( index < phrases.size() - 1 )
					{
						Phrase * rightPhrase = phrases[index+1];
						long offset = rightPhrase->GetStart() - curPhrase->GetEnd();
						curPhrase->SetEnd(InvTranslate(Translate(curPhrase->GetEnd()) - 1));
						rightPhrase->SetStart(curPhrase->GetEnd() + offset);
					}
					else 
					{
						curPhrase->SetEnd(InvTranslate(Translate(curPhrase->GetEnd()) - 1));
					}
				}
			}
			//передвижение только левой границы (без граница соседа) влево/вправо
			else if ( event->modifiers() == (Qt::AltModifier | Qt::ShiftModifier) )
			{
				if ( !is_edit_disable)
					curPhrase->SetStart(InvTranslate(Translate(curPhrase->GetStart()) - 1));
			}
			//передвижение только правой границы (без граница соседа) влево/вправо
			else if ( event->modifiers() == (Qt::AltModifier | Qt::ControlModifier) )
			{
				if ( !is_edit_disable)
					curPhrase->SetEnd(InvTranslate(Translate(curPhrase->GetEnd()) - 1));
			}
			else if ( index > 0 )
			{
				SetCurPhrase(phrases[index-1]);
			}
		}
		else if ( event->key() == Qt::Key_Right )
		{
			//передвижение левой границы (+ граница соседа) вправо
			if ( event->modifiers() == Qt::ShiftModifier )
			{
				if ( !is_edit_disable)
				{
					if ( index > 0 )
					{
						Phrase * leftPhrase = phrases[index-1];
						long offset = curPhrase->GetStart() - leftPhrase->GetEnd();
						curPhrase->SetStart(InvTranslate(Translate(curPhrase->GetStart()) + 2));
						leftPhrase->SetEnd(curPhrase->GetStart() - offset);
					}
					else
						curPhrase->SetStart(InvTranslate(Translate(curPhrase->GetStart()) + 2));			
				}
			}
			//передвижение правой границы (+ граница соседа) вправо
			else if ( event->modifiers() == Qt::ControlModifier )
			{
				if ( !is_edit_disable)
				{
					if ( index < phrases.size() - 1 )
					{
						Phrase * rightPhrase = phrases[index+1];
						long offset = rightPhrase->GetStart() - curPhrase->GetEnd();
						rightPhrase->SetStart(InvTranslate(Translate(rightPhrase->GetStart()) + 2));
						curPhrase->SetEnd(rightPhrase->GetStart() - offset);
					}
					else 
					{
						curPhrase->SetEnd(InvTranslate(Translate(curPhrase->GetEnd()) + 2));
					}
				}
			}
			//передвижение только левой границы (без граница соседа) влево/вправо
			else if ( event->modifiers() == (Qt::AltModifier | Qt::ShiftModifier) )
			{
				if ( !is_edit_disable)
					curPhrase->SetStart(InvTranslate(Translate(curPhrase->GetStart()) + 2));
			}
			//передвижение только правой границы (без граница соседа) влево/вправо
			else if ( event->modifiers() == (Qt::AltModifier | Qt::ControlModifier) )
			{
				if ( !is_edit_disable)
					curPhrase->SetEnd(InvTranslate(Translate(curPhrase->GetEnd()) + 2));
			}
			else if ( index < phrases.size()-1 )
			{
				SetCurPhrase(phrases[index+1]);
			}		
		}
		//! Обновим курсор
		else if ( event->key() == Qt::Key_Control )
		{
			QPoint offset;
			UpdateCursorShape(GetAction(curPhrase, curPos, offset));
		}
	}
}

void PhrasesListData::keyReleaseEvent(QKeyEvent * event)
{
	if ( event->key() == Qt::Key_Control && action == NoAction )
	{
		if ( GetPhrase(curPos) )
			setCursor(Qt::PointingHandCursor);
		else 
			setCursor(Qt::ArrowCursor);
	}
}

void PhrasesListData::wheelEvent(QWheelEvent * event)
{
	const QRectF & rect = controller->GetWindow();
	emit ScaleHRuller((event->pos().x()/(qreal)width()) * rect.width() + rect.x(), event->delta());
	setFocus();
}

void PhrasesListData::OnEditingFinished()
{
	if ( lineEdit )
	{
		qDebug() << "PhrasesListData::OnEditingFinished()";
		lineEdit->hide();
		setFocus();
	}
}

void PhrasesListData::OnReturnPressed()
{
	if ( lineEdit && curPhrase )
	{
		curPhrase->SetName(lineEdit->text());
		lineEdit->hide();
		blockEdit = true;
	}
}

void PhrasesListData::OnTextChanged(const QString & text)
{
	if ( curPhrase )
	{
		curPhrase->SetName(text);
		blockEdit = true;
	}
}


long PhrasesListData::InvTranslate(long pos )
{
	const QRectF & window = controller->GetWindow();
	return (pos/(qreal)width() * window.width() + window.left()) + 0.5;
}

long PhrasesListData::Translate(long sample)
{
	const QRectF & window = controller->GetWindow();
	return (sample - window.left())/window.width() * width();
}

QRect PhrasesListData::GetPhraseRect(const Phrase * phrase)
{
	if ( phrase )
	{
		const QRectF & window = controller->GetWindow();
		int x1 = Translate(phrase->GetStart());
		int x2 = Translate(phrase->GetEnd());
		QRect r(x1, 0, x2-x1, height());

		if ( r.width() >= selectedWidth * 2)
			return r;
		return QRect(r.left()-selectedWidth, r.top(), r.width() + 2*selectedWidth, r.height());
	}
	return QRect();
}