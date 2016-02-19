#include "waveviewerwidget.h"
#include <QPainter>
#include <iostream>
#include <QMoveEvent>
#include <assert.h>
#include <math.h>
#include "buttonscontroller.h"
#include "selectbutton.h"
#include "scrollbutton.h"
#include <QTime>
#include <QTimer>
#include <QDragEnterEvent>
#include <QUrl>

#include "../phrasecontroller/phrasecontroller.h"
#include "../phrasecontroller/phraseslist/phraseslist.h"
#include "../phrasecontroller/phrasebutton/phrasebutton.h"
#include "../phrasecontroller/phrase/phrase.h"

#include "frame/dataframe/waveframe/waveframe.h"
#include "frame/dataframe/spectorframe/spectorframe.h"
#include "frame/hrullerframe/hrullerframe.h"
#include "frame/vrullerframe/vrullerframe.h"
#include <QMenu>
#include "../phrasecontroller/phraseslist/phraseslistdata/phraseslistdata.h"
#include <QDebug>

WaveViewerWidget::WaveViewerWidget(QWidget *parent)
	: QFrame(parent),
	vRullerFrame(new VRullerFrame),
	hRullerFrame(new HRullerFrame),
	data(new WaveFrame(samples)),
	window(-1, -3, 6, 6),
	curPos(0,0),
	mouseState(0),
	startPos(0,0),
	enableSelectWindow(true),
	enableTanslate(true),
	enableSelectSamples(true),
	enableMoveControl(true),
	startSample(0),
	endSample(0),
	start_select_sample(0),
	end_select_sample(0),
	maxWindow(-1, -3, 6, 6),
	minWindowSize(4.,4.),
	leftMargin(4),
	rightMargin(4),
	topMargin(4),
	bootomMargin(4),
	hRullerHigth(20),
	vRullerWidth(50),
	scrollHeight(14),
	controllsHeight(10),
	curAction(NoAction),
	currentPlayPos(0),
	visibleCurrentPlayPos(true),
	curButton(NULL),
	controller(NULL),
	showWave(true),
	menu_view(NULL),
	menu_edit(NULL),
	playPosTracking(false),
	key_repeat_count(0),
	is_selected(true),
	is_h_ruller_visible(true),
	is_scroll_visible(true),
	is_controlls_height_visible(true)
{
	UpdateScheme();
	markingController = new ButtonsController(*this);
	selectorController = new ButtonsController(*this);
	scrollController = new ButtonsController(*this);

	startSelector = new SelectButton(selectorController, *this, true);
	endSelector = new SelectButton(selectorController, *this, false);
	startSelector->SetNeighbor(endSelector);
	endSelector->SetNeighbor(startSelector);


	scrollButton = new ScrollButton(scrollController, *this);

	moveTimer = new QTimer();
	moveTimer->setSingleShot(false);
	moveTimer->setInterval(10);
	connect(moveTimer, SIGNAL(timeout()), this, SLOT(OnMoveTimeout()));
	connect(data, SIGNAL(DataChanged(long)), this, SLOT(UpdateWindows()));
	connect(data, SIGNAL(DataChanged(long)), this, SIGNAL(DataChanged(long)));

	connect(data, SIGNAL(ImageUpdated()), this, SLOT(OnUpdate()));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(OnCustomContextMenuRequested(const QPoint &)));

	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus); 
	setAcceptDrops(true);

	UpdateWindows();

	EnableSelectSamples(true);
	EnableSelectWindow(false);
	SetVisibleCurrentPlayPos(false);
}

WaveViewerWidget::~WaveViewerWidget()
{
	delete markingController;
	delete selectorController;
	delete scrollController;
	delete moveTimer;
	delete vRullerFrame;
	delete hRullerFrame;
	delete data;
}



bool WaveViewerWidget::SetWindow(const QRectF & _w)
{
	QRectF w = _w;
	if ( w.width() > maxWindow.width() )
		w.setWidth(maxWindow.width());
	else if ( w.width() < minWindowSize.width() )
		w.setWidth(minWindowSize.width());

	if ( w.height() > maxWindow.height() )
		w.setHeight(maxWindow.height());
	else if ( w.height() < minWindowSize.height())
		w.setHeight(minWindowSize.height());


	if ( w != window )
	{
		if ( maxWindow.intersected(w) == w )
			window = w;
		//Смещаем.
		else 
		{
			QPointF pos = w.topLeft();

			if ( w.left() < maxWindow.left() )
				pos.rx() = maxWindow.left();

			if ( w.right() > maxWindow.right() )
				pos.rx() = maxWindow.right() - w.width();

			if ( w.top() < maxWindow.top() )
				pos.ry() = maxWindow.top();

			if ( w.bottom() > maxWindow.bottom())
				pos.ry() = maxWindow.bottom() - w.height();

			window.setRect(pos.x(), pos.y(), w.width(), w.height());
		}

		update();
		data->SetWindow(window);
		vRullerFrame->SetWindow(window);
		hRullerFrame->SetWindow(window);
		hRullerFrame->SetSampleRate(data->GetSampleRate());

		//! Позиция изменилась
		QPointF v = Transform(curPos);
		emit PosChanged(v.x(), v.y());
		emit WindowChanged(window);

		return true;
	}
	return false;
}

const QRectF & WaveViewerWidget::GetWindow() const
{
	return window;
}


const QRectF & WaveViewerWidget::GetMaxWindow() const
{
	return maxWindow;
}

const QSizeF & WaveViewerWidget::GetMinWindowSize() const
{
	return minWindowSize;
}



QRectF WaveViewerWidget::GetSelectedWindow()
{
	QRect plotRect = GetPlotRect();
	QRect selectedRect = GetSelectedRect();

	qreal x = selectedRect.left()/(qreal)plotRect.width();
	qreal y = (plotRect.height() - selectedRect.bottom())/(qreal)plotRect.height();
	qreal width = selectedRect.width()/(qreal)plotRect.width();
	qreal height = selectedRect.height()/(qreal)plotRect.height();
	return QRectF(window.x() + x * window.width(),
				  window.y() + y * window.height(),
				  window.width() * width,
				  window.height() * height);
}


void WaveViewerWidget::EnableSelectWindow(bool enable)
{
	enableSelectWindow = enable;
	update();
}

bool WaveViewerWidget::IsEnableSelectWindow()
{
	return enableSelectWindow;
}

void WaveViewerWidget::EnableTranslate(bool enable)
{
	enableTanslate = enable;
	update();
}

bool WaveViewerWidget::IsEnableTranslate() const
{
	return enableTanslate;
}

void WaveViewerWidget::EnableSelectSamples(bool enable)
{
	enableSelectSamples = enable;
	update();
}

bool WaveViewerWidget::IsEnableSelectSamples() const
{
	return enableSelectSamples;
}

QPointF WaveViewerWidget::Transform(const QPoint & pos, bool offset)
{
	QRect plotRect = GetPlotRect();
	if ( !offset )
	{
		qreal x = (pos.x() - plotRect.left())/(qreal)plotRect.width();
		qreal y = (plotRect.bottom() - pos.y())/(qreal)plotRect.height();
		return QPointF(window.x() + x * window.width(), window.y() + y * window.height());
	}
	else 
	{
		qreal x = pos.x() / (qreal)plotRect.width();
		qreal y = pos.y() / (qreal)plotRect.height();
		return QPointF(x * window.width(), y * window.height());
	}
}

QPoint WaveViewerWidget::InvTansform(const QPointF & pos)
{
	qreal x = (pos.x() - window.x())/window.width();
	qreal y = (pos.y() - window.y())/window.height();
	QRect plotRect = GetPlotRect();
	return QPoint(x * plotRect.width() + plotRect.left(), plotRect.bottom() - y * plotRect.height());
}

bool WaveViewerWidget::SelectSamples(long s, long e, bool autoScale)
{
	if ( !data->GetCount() )
	{
		s = 0;
		e = 0;
	}
	else 
	{
		if ( s < 0 )
			 s = 0;
		else if ( s > data->GetCount() )
			s = data->GetCount();
		if ( e < s  )
			e = s;
		else if ( e > data->GetCount() )
			e = data->GetCount();
	}

	if ( s != startSample || 
		e != endSample )
	{
		startSample = s;
		endSample = e;


		EmitSamplesChanged();
		update();

		if ( autoScale )
			CheckSelected();
	}
	return true;
}

void WaveViewerWidget::CheckSelected()
{
	qreal sampleStart = startSample;
	qreal sampleWidth = (endSample - sampleStart)/2.;
	qreal sampleCenter = sampleStart + sampleWidth;

	qreal windowWidth = window.width()/2.;

	//Размер выделенного больше чем окно.
	if ( sampleWidth > windowWidth )
	{
		QRectF w(sampleStart, window.top(), sampleWidth*2, window.height());
		SetWindow(w);
	}
	else 
	{
		qreal offset = sampleCenter - window.center().x();
		
		//Выход за границы окна.
		if ( fabs(offset) + sampleWidth >  windowWidth )
		{
			QRectF w = window;
			//выделенное правее окна.
			if ( offset > 0 )
			{
				w.moveRight(sampleCenter + sampleWidth);
				SetWindow(w);
			}
			//выделенное левее окна.
			else 
			{
				w.moveLeft(sampleCenter - sampleWidth);
				SetWindow(w);			
			}
		}
	}
}


QPair<long, long> WaveViewerWidget::GetSelectedSamples() const
{
	return QPair<long, long>(startSample,endSample);
}



QPoint WaveViewerWidget::FromMouseToPlot(const QPoint & pos) const
{
	QRect r = GetPlotRect();
	return QPoint(pos.x() - r.x(), pos.y() - r.y());
}

QRect WaveViewerWidget::GetSelectedRect() const
{
	QRect r = GetPlotRect();
	return QRect(qMin(startPos.x(), curPos.x()) - r.left(),
				 qMin(startPos.y(), curPos.y()) - r.top(),
				 abs(startPos.x() - curPos.x()),
				 abs(startPos.y() - curPos.y()));
}

const QRect & WaveViewerWidget::GetPlotRect() const
{
	return plotRect;
}

const QRect & WaveViewerWidget::GetHRullerRect() const
{
	return hRullerRect;
}

const QRect & WaveViewerWidget::GetVRullerRect() const
{
	return vRullerRect;
}

const QRect & WaveViewerWidget::GetControlsRect() const
{
	return controllRect;
}

const QRect & WaveViewerWidget::GetScrollRect() const
{
	return scrollRect;
}

bool WaveViewerWidget::SetCurrentPlayPos(long pos)
{
	if ( pos != currentPlayPos )
	{
		if ( pos < 0)
			pos = 0;

		if ( pos >= data->GetCount() )
			pos = data->GetCount() - 1;
		

		//! Смещаем окно относительно курсора.
		if ( playPosTracking )
		{
			long offset = pos  - currentPlayPos;
			QRectF w = GetWindow();
			w.moveLeft(w.left() + offset);
			SetWindow(w);
		}

		currentPlayPos  = pos;
		update();
		//std::cout << "pos:" << currentPlayPos << std::endl;
		return true;
	}
	return false;
}

long WaveViewerWidget::GetCurrentPlayPos() const
{
	return currentPlayPos;
}

void WaveViewerWidget::SetVisibleCurrentPlayPos(bool visible)
{
	if ( visibleCurrentPlayPos != visible)
	{
		visibleCurrentPlayPos = visible;
		update();
	}
}

bool WaveViewerWidget::IsVisibleCurrentPlayPos() const
{
	return visibleCurrentPlayPos;
}


void WaveViewerWidget::paintEvent( QPaintEvent * event )
{
	QFrame::paintEvent(event);
	if ( window.isValid() )
	{
		QPainter painter(this);

		painter.drawPixmap(plotRect.topLeft(), data->GetPixmap());
		painter.drawPixmap(vRullerRect.topLeft(), vRullerFrame->GetPixmap());
		painter.drawPixmap(hRullerRect.topLeft(), hRullerFrame->GetPixmap());

		//Нарисуем сетку.
		painter.setClipRect(plotRect);
		painter.setPen(QColor(0, 53, 0));

		//горизонтальная
		foreach(int y, vRullerFrame->GetMarks() )
		{
			int yy = vRullerRect.top() + y;
			painter.drawLine( plotRect.left(), yy, plotRect.right(), yy);
		}
		//вертикальная
		foreach(int x, hRullerFrame->GetMarks() )
		{
			int xx = hRullerRect.left() + x;
			painter.drawLine(xx, plotRect.top(), xx, plotRect.bottom());
		}

		//Рисуем 0;
		painter.setPen(Qt::red);
		int y = plotRect.bottom() - (-window.y()/window.height() * plotRect.height());
		painter.drawLine( plotRect.left(), y, plotRect.right(), y);


		//Разметка.
		DrawMarking(painter);

		//Выделенное 
		DrawSelectedSamples(painter);

		DrawScroll(painter);

		DrawSelectors(painter);

		//! Позиция проигрывания.
		DrawPlayPos(painter);

		if ( !is_selected )
		{
			painter.setClipRect(plotRect);
			painter.setCompositionMode(QPainter::CompositionMode_Lighten);
			painter.fillRect(plotRect, QBrush(QColor(0,0,100)));
		}
	}
}


void WaveViewerWidget::mouseMoveEvent ( QMouseEvent * event )
{
	//Смещаем картинку
	if ( enableTanslate && event->buttons() & Qt::MidButton )
	{
		QPointF offset = Transform(event->pos() - curPos, true);
		offset.rx() = offset.x() * -1.;

		QRectF rect = GetWindow();
		rect.moveCenter(offset + rect.center());
		SetWindow(rect);
	}
	else if ( (event->buttons() & Qt::LeftButton) )
	{
		if ( curAction == SelectActon &&  enableSelectSamples )
		{
			end_select_sample = ToSelectedSample(Transform(event->pos()).x());
			if ( end_select_sample < 0 )
				end_select_sample = 0;
			//std::cout << "curSample: " << curSample << "\n";
			SelectSamples(qMin(end_select_sample, start_select_sample), qMax(end_select_sample, start_select_sample), false);

			//Смещаем окно.
			if ( event->pos().x() > plotRect.right() )
			{
				offset.rx() = Transform(event->pos() - plotRect.topRight(), true).x();
				if ( !moveTimer->isActive() )
					moveTimer->start();

			}
			else if ( event->pos().x() < plotRect.left() )
			{
				offset.rx() = Transform(event->pos() - plotRect.topLeft(), true).x();
				if ( !moveTimer->isActive() )
					moveTimer->start();
			}
			else if ( moveTimer->isActive() )
				moveTimer->stop();

		}
		else if ( curAction == MoveControls && curButton )
		{
			if ( curButton)
				curButton->Move(event);
		}
		else if ( curAction == MoveVertical )
		{
			if ( event->pos().y() > vRullerRect.bottom() )
			{
				offset.ry() = Transform(event->pos() - vRullerRect.bottomLeft(), true).y();
				
				if ( !moveTimer->isActive() )
					moveTimer->start();

			}
			else if ( event->pos().y() < vRullerRect.top() )
			{
				offset.ry() = Transform(event->pos() - vRullerRect.topLeft(), true).y();
				if ( !moveTimer->isActive() )
					moveTimer->start();
			}
			else 
			{
				offset.ry() = Transform(event->pos()- curPos, true).y();
				QRectF rect = GetWindow();
				rect.moveCenter(rect.center() + offset);
				SetWindow(rect);

				if ( moveTimer->isActive() )
					moveTimer->stop();
			}
		}
		else if ( curAction == MoveHorizontal )
		{
			if ( event->pos().x() > hRullerRect.right() )
			{
				offset.rx() = Transform(hRullerRect.topRight() - event->pos(), true).x();
				if ( !moveTimer->isActive() )
					moveTimer->start();

			}
			else if ( event->pos().x() < hRullerRect.left() )
			{
				offset.rx() = Transform(hRullerRect.topLeft() - event->pos(), true).x();
				if ( !moveTimer->isActive() )
					moveTimer->start();

			}
			else 
			{
				offset = Transform(curPos - event->pos(), true);
				QRectF rect = GetWindow();
				rect.moveCenter(rect.center() + offset);
				SetWindow(rect);
				if ( moveTimer->isActive() )
					moveTimer->stop();
				
			}
		}	
	}


	if ( GetButton(event->pos()) )
		setCursor(Qt::PointingHandCursor);
	else
		setCursor(Qt::ArrowCursor);

	//! Позиция изменилась
	QPointF v = Transform(event->pos());
	emit PosChanged(v.x(), v.y());

	curPos = event->pos();
	update();
}

void WaveViewerWidget::mousePressEvent ( QMouseEvent * event )
{
	mouseState = event->buttons();
	QPoint pos = event->pos();

	if ( event->button() == Qt::LeftButton )
	{
		startPos = pos;
		curAction = NoAction;

		//Начало перемещения контрола.
		if ( enableMoveControl && (curButton = SelectAndPressButton(event)) )
		{
			bool isPhraseButton = dynamic_cast<PhraseButton *>(curButton) != NULL;

			if ( isPhraseButton && (event->modifiers() & Qt::ControlModifier) || !isPhraseButton )
				curAction = MoveControls;
		}
		//Начало выделения
		else if ( enableSelectSamples && GetPlotRect().contains(pos) )
		{
			start_select_sample = ToSelectedSample(Transform(startPos).x());
			end_select_sample = start_select_sample;
			SelectSamples(start_select_sample, end_select_sample, false);
			curAction = SelectActon;
		}
		else if ( GetVRullerRect().contains(pos) )
		{
			curAction = MoveVertical;
		}
		else if ( GetHRullerRect().contains(pos) )
		{
			curAction = MoveHorizontal;
		}
	}
	curPos = pos;
	update();

	emit MousePress(event);
}

void WaveViewerWidget::mouseReleaseEvent ( QMouseEvent * event )
{
	event->pos();
	mouseState = event->buttons();

	if ( event->button() == Qt::LeftButton )
	{
		if ( enableSelectWindow )
			emit SelectWindow(GetSelectedWindow());

		if ( enableMoveControl &&  curAction == MoveControls &&	curButton )
			curButton->MouseRelease(event);

		startPos = event->pos();
		curAction = NoAction;
		offset = QPointF();
		if ( moveTimer->isActive() )
			moveTimer->stop();
	}
	curPos = event->pos();
	update();

	emit MouseRelease(event);
}

void WaveViewerWidget::ScaleHRuller(qreal hWindowCenter, int delta)
{
	if ( enableTanslate )
	{
		//Масштабируем относительно курсора.
		qreal left = window.left() - hWindowCenter;
		qreal right = window.right() - hWindowCenter;
		
		if ( delta < 0 )
		{
			left = left * 1.5;
			right = right * 1.5;
		}
		else 
		{
			left = left / 1.5;
			right = right / 1.5;
		}
		SetWindow(QRectF(hWindowCenter + left, window.top(), right - left, window.height()));	
	}
}


void WaveViewerWidget::ScaleVRuller(qreal vWindowCenter, int delta)
{
	if ( enableTanslate )
	{
		//Масштабируем относительно курсора.
		qreal top = window.top() - vWindowCenter;
		qreal bottom = window.bottom() - vWindowCenter;
		
		if ( delta < 0 )
		{
			top = top * 1.5;
			bottom = bottom * 1.5;
		}
		else 
		{
			top = top / 1.5;
			bottom = bottom / 1.5;
		}
		SetWindow(QRectF(window.left(), vWindowCenter + top, window.width(),  bottom - top));	
	}
}



void WaveViewerWidget::wheelEvent(QWheelEvent * event)
{
	curPos = event->pos();

	if ( enableTanslate )
	{
		//Масштабируем относительно курсора.
		QPointF center = Transform(curPos);
		if ( vRullerRect.contains(event->pos()) )
			ScaleVRuller(center.y(), event->delta());
		else 
			ScaleHRuller(center.x(), event->delta());
	}
}

void WaveViewerWidget::keyPressEvent(QKeyEvent * event)
{
	//увеличивает скорость перемещения позиции
	if (event->isAutoRepeat() == false)
		key_repeat_count = 0;
	else
	{
		key_repeat_count++;
		if (key_repeat_count > 50)
			key_repeat_count = 50;
	}
	
	qDebug() << "key_repeat_count:" << key_repeat_count <<  " modifers:" << event->modifiers() << "\n";

	
	//Начало процесса выделения.
	if ( event->key() == Qt::Key_Left )
	{
		//Функция масштабирования.
		if ( event->modifiers() & Qt::ControlModifier )
		{
			ZoomInHorizontal();
		}
		//Функция выделения.
		else if ( event->modifiers() & Qt::ShiftModifier )
		{
			//секунды
			QPointF start((qreal)end_select_sample, 0);
			QPoint point = InvTansform(start);
			point.rx() = point.x() -1 - key_repeat_count;
			QPointF end = Transform(point);
			end_select_sample = ToSelectedSample(end.x());
			SelectSamples(qMin(end_select_sample, start_select_sample), qMax(end_select_sample, start_select_sample), false);
		}
		//Функция перемещения
		else
		{
			//секунды
			QPointF start((qreal)startSample, 0);
			QPoint point = InvTansform(start);
			point.rx() = point.x() -1 - key_repeat_count;
			QPointF end = Transform(point);
			start_select_sample = ToSelectedSample(end.x());
			start_select_sample = start_select_sample != startSample ? start_select_sample : start_select_sample-1;
			end_select_sample =  start_select_sample;

			SelectSamples(start_select_sample, end_select_sample, true);
		}
	}
	else if ( event->key() == Qt::Key_Right )
	{
		//Функция масштабирования.
		if ( event->modifiers() & Qt::ControlModifier )
		{
			ZoomOutHorizontal();
		}
		//Функция выделения.
		else if ( event->modifiers() & Qt::ShiftModifier )
		{
			QPointF start((qreal)end_select_sample, 0);
			QPoint point = InvTansform(start);
			point.rx() = point.x() + 2 + key_repeat_count;
			QPointF end = Transform(point);
			end_select_sample = ToSelectedSample(end.x());
			end_select_sample = end_select_sample != startSample ? end_select_sample : end_select_sample + 1;
			SelectSamples(qMin(end_select_sample, start_select_sample), qMax(end_select_sample, start_select_sample), false);
		}
		//Функция перемещения
		else
		{
			//секунды.
			QPointF start((qreal)startSample, 0);
			QPoint point = InvTansform(start);
			point.rx() = point.x() + 2 + key_repeat_count;
			QPointF end = Transform(point);
			start_select_sample = ToSelectedSample(end.x());
			start_select_sample = start_select_sample != startSample ? start_select_sample : start_select_sample + 1;
			end_select_sample = start_select_sample;
			SelectSamples(start_select_sample, end_select_sample, true);
		}
	}
	else if ( event->key() == Qt::Key_Up )
	{
		//Функция масштабирования.
		if ( event->modifiers() & Qt::ControlModifier )
		{
			ZoomOutVertical();
		}
	}
	else if ( event->key() == Qt::Key_Down )
	{
		//Функция масштабирования.
		if ( event->modifiers() & Qt::ControlModifier )
		{
			ZoomInVertical();
		}
	}
}


void WaveViewerWidget::MoveLeft_LeftMarkerBorder()
{
	//секунды.
	QPointF start((qreal)startSample, 0);
	QPoint point = InvTansform(start);
	point.rx() = point.x() - 1;
	QPointF end = Transform(point);
	long v = ToSelectedSample(end.x());
	SelectSamples(v,endSample, true);
}

void WaveViewerWidget::MoveRight_LeftMarkerBorder()
{
	//секунды.
	QPointF start((qreal)startSample, 0);
	QPoint point = InvTansform(start);
	point.rx() = point.x() + 2;
	QPointF end = Transform(point);
	long v = ToSelectedSample(end.x());
	v = v > endSample ? endSample : v;
	SelectSamples(v,endSample, true);
}

void WaveViewerWidget::ZoomInVertical()
{
	QRectF rect = GetWindow();
	QPointF center(rect.center());
	rect.setHeight(rect.height() * 0.9);
	rect.moveCenter(center);
	SetWindow(rect);
}

void WaveViewerWidget::ZoomOutVertical()
{
	QRectF rect = GetWindow();
	QPointF center(rect.center());
	rect.setHeight(rect.height() * 1.1);
	rect.moveCenter(center);
	SetWindow(rect);
}

void WaveViewerWidget::ZoomInHorizontal()
{
	QRectF rect = GetWindow();
	QPointF center(rect.center());
	rect.setWidth(rect.width() * 0.9);
	rect.moveCenter(center);
	SetWindow(rect);
}

void WaveViewerWidget::ZoomOutHorizontal()
{
	QRectF rect = GetWindow();
	QPointF center(rect.center());
	rect.setWidth(rect.width() * 1.1);
	rect.moveCenter(center);
	SetWindow(rect);
}

void WaveViewerWidget::setHRullerVisible(bool visible)
{
	if ( is_h_ruller_visible != visible )
	{
		is_h_ruller_visible = visible;
		update();
	}
}

void WaveViewerWidget::setScrollVisible(bool visible)
{
	if ( is_scroll_visible != visible )
	{
		is_scroll_visible = visible;
		update();
	}
}

void WaveViewerWidget::setControllsHeightVisible(bool visible)
{
	if ( is_controlls_height_visible != visible )
	{
		is_controlls_height_visible = visible;
		update();
	}
}

void WaveViewerWidget::MoveLeft_RightMarkerBorder()
{
	//секунды.
	QPointF start((qreal)endSample, 0);
	QPoint point = InvTansform(start);
	point.rx() = point.x() - 1;
	QPointF end = Transform(point);
	long v = ToSelectedSample(end.x());
	v = v < startSample ? startSample : v;
	SelectSamples(startSample, v, true);
}

void WaveViewerWidget::MoveRight_RightMarkerBorder()
{
	//секунды.
	QPointF start((qreal)endSample, 0);
	QPoint point = InvTansform(start);
	point.rx() = point.x() + 2;
	QPointF end = Transform(point);
	long v = ToSelectedSample(end.x());
	SelectSamples(startSample, v, true);
}




void WaveViewerWidget::resizeEvent(QResizeEvent * event)
{
	UpdateScheme();
	QWidget::resizeEvent(event);
}


void WaveViewerWidget::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void WaveViewerWidget::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void WaveViewerWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void WaveViewerWidget::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls()) 
	{
		QList<QUrl> urlList = mimeData->urls();
		QString text;
		for (int i = 0; i < urlList.size() && i < 32; ++i) 
		{
			emit DropFile(urlList[i].toLocalFile());
			//break;
		}
		text = text;
    }
	event->acceptProposedAction();
}



void WaveViewerWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	curPos = event->pos();

	if ( enableSelectSamples )
	{
		if ( GetPlotRect().contains(curPos) )
		{
			SelectSamples(0, data->GetCount(), false);
		}
	}
}


void WaveViewerWidget::UpdateScheme()
{
	int width = this->width();
	int height = this->height();
	
	scrollRect.setRect(leftMargin, topMargin, width - leftMargin - rightMargin - vRullerWidth, (is_scroll_visible ? scrollHeight : 0));
	controllRect.setRect(leftMargin, height - bootomMargin - (is_controlls_height_visible ? controllsHeight : 0), scrollRect.width(), (is_controlls_height_visible ? controllsHeight : 0));

	hRullerRect.setRect(leftMargin, controllRect.top() - 2 - (is_h_ruller_visible ? hRullerHigth : 0), controllRect.width(), (is_h_ruller_visible ? hRullerHigth : 0));
	vRullerRect.setRect(width - rightMargin - vRullerWidth, scrollRect.bottom()+2, vRullerWidth, hRullerRect.top() - scrollRect.bottom()-2);
	plotRect.setRect(leftMargin, vRullerRect.top(), hRullerRect.width(), vRullerRect.height());

	data->SetSize(plotRect.size());
	vRullerFrame->SetSize(vRullerRect.size());
	hRullerFrame->SetSize(hRullerRect.size());
	hRullerFrame->SetSampleRate(data->GetSampleRate());
}

void WaveViewerWidget::DrawSelectedWindow(QPainter & painter)
{
	//Рисуем выделялку.
	if ( enableSelectWindow && mouseState & Qt::LeftButton )
	{
		painter.setPen(Qt::white);
		painter.setBrush(Qt::NoBrush);
		QRect plotRect  = GetPlotRect();
		QRect selectedRect = GetSelectedRect();

		painter.drawRect(selectedRect.x() + plotRect.x(),
			selectedRect.y() + plotRect.y(), 
			selectedRect.width(),
			selectedRect.height());
	}
}

void WaveViewerWidget::DrawSelectedSamples(QPainter & painter)
{
	if ( enableSelectSamples )
	{
		QRect plotRect = GetPlotRect();

		painter.setPen(QColor(255,255,255));
		painter.setBrush(QColor(255,255,255,100));
		painter.setClipRect(plotRect);

		qreal offset = 0.5;
		int x1 = plotRect.left() + plotRect.width() * (startSample  - offset - window.left())/ window.width();
		int x2 = plotRect.left() + plotRect.width() * (endSample - offset - window.left())/ window.width();
		painter.drawRect( QRect(x1, plotRect.y(), x2 - x1, plotRect.height()) );
	}
}


void WaveViewerWidget::EmitSamplesChanged()
{
	emit SelectedSamplesChanged(startSample, endSample);
}


void WaveViewerWidget::DrawPlayPos(QPainter & painter)
{
	if ( visibleCurrentPlayPos )
	{
		QRect plotRect = GetPlotRect();
		painter.setClipRect(plotRect);
		int x = InvTansform(QPointF(currentPlayPos, 0)).x();

		painter.setPen(Qt::white);
		painter.drawLine(x, plotRect.top(), x, plotRect.bottom()); 
	}
}

inline bool HasProjection2(int sA, int eA, int sB, int eB)
{
	int aL = (eA - sA) >> 1;
	int bL = (eB - sB) >> 1;
	int offsetAB = (sA + aL) - (sB + bL);
	return abs(offsetAB) < aL + bL;
}

void WaveViewerWidget::DrawMarking(QPainter & painter)
{
	if ( controller )
	{
		const QList<PhrasesList *> & lists = controller->GetLists();
		painter.setClipRect(plotRect);

		foreach(PhrasesList * list, lists)
		{
			if ( list->IsEnable() )
			{
				QPen pen(list->GetData().GetColor());
				pen.setStyle(Qt::DashDotLine);
				painter.setPen(pen);				
				int last;
				bool init = false;

				const QList<Phrase *> & phrases = list->GetData().GetPhrases();
				foreach(Phrase * data, phrases)
				{
					int x1 = (data->GetStart() - window.left())/window.width() * plotRect.width() + plotRect.left();
					int x2 = (data->GetEnd() - window.left())/window.width() * plotRect.width() + plotRect.left();

					if ( HasProjection2(x1, x2, plotRect.left(), plotRect.right()) )
					{
						if ( !init || x1 != last )
							painter.drawLine(x1, plotRect.top(), x1, plotRect.bottom());

						if ( !init || x2 != last )
							painter.drawLine(x2, plotRect.top(), x2, plotRect.bottom());
						init = true;
						last  = x2;
					}

				}
			}
		}
	}
}

void WaveViewerWidget::DrawScroll(QPainter & painter)
{
	//Фон.
	painter.setClipRect(GetScrollRect());
	painter.setPen(Qt::black);
	painter.setBrush(Qt::black);
	painter.drawRect(GetScrollRect());

	//Слайдер.
	QPen pen(Qt::SolidLine);
	pen.setColor(Qt::white);
	painter.setPen(pen);
	painter.setBrush(QColor(75, 243, 167));
	painter.drawRect(scrollButton->GetDisplayedRect());

	////! Выделение.
	//painter.setPen(Qt::white);
	//painter.setBrush(QColor(255,255,255,100));
	//painter.drawRect(GetSelectedRect());
}



void WaveViewerWidget::DrawSelectors(QPainter & painter)
{
	painter.setClipRect(controllRect);
	painter.setPen(Qt::black);
	painter.setBrush(Qt::black);
	painter.drawRect(controllRect);


	QPen pen(Qt::SolidLine);
	pen.setColor(QColor(255,255,0));
	painter.setPen(pen);
	painter.setBrush(pen.color());


	QPoint startControl[3];
	QRect rect = startSelector->GetRect();
	
	if ( startSelector->IsStart() )
	{
		startControl[0] = QPoint(rect.left(), rect.top());
		startControl[1] = QPoint(rect.right(), rect.bottom());
		startControl[2] = QPoint(rect.left(), rect.bottom());
		painter.drawPolygon(startControl, sizeof(startControl)/sizeof(startControl[0]));
	}
	else 
	{
		startControl[0] = QPoint(rect.right(), rect.top());
		startControl[1] = QPoint(rect.right(), rect.bottom());
		startControl[2] = QPoint(rect.left(), rect.bottom());
		painter.drawPolygon(startControl, sizeof(startControl)/sizeof(startControl[0]));	
	}

	rect = endSelector->GetRect();

	if ( !endSelector->IsStart() )
	{
		startControl[0] = QPoint(rect.right(), rect.top());
		startControl[1] = QPoint(rect.right(), rect.bottom());
		startControl[2] = QPoint(rect.left(), rect.bottom());
		painter.drawPolygon(startControl, sizeof(startControl)/sizeof(startControl[0]));
	}
	else 
	{
		startControl[0] = QPoint(rect.left(), rect.top());
		startControl[1] = QPoint(rect.right(), rect.bottom());
		startControl[2] = QPoint(rect.left(), rect.bottom());
		painter.drawPolygon(startControl, sizeof(startControl)/sizeof(startControl[0]));
	}
}

long WaveViewerWidget::ToSample(qreal sec) const 
{
	return sec/(1./data->GetSampleRate());
}

qreal WaveViewerWidget::ToSec(long sample) const
{
	return sample /(qreal)data->GetSampleRate();
}

void WaveViewerWidget::DisplaySamples(bool samples)
{
	hRullerFrame->ShowSamples(samples);
}

bool WaveViewerWidget::IsDisplaySamples() const
{
	return hRullerFrame->IsShowSamples();
}

bool WaveViewerWidget::ShowWave(bool wave)
{
	if ( showWave != wave )
	{
		DataFrame * oldData = data;

		if ( wave )
			data = new WaveFrame(samples);
		else 
			data = new SpectorFrame(samples);

		connect(data, SIGNAL(DataChanged(long)), this, SLOT(UpdateWindows()));
		connect(data, SIGNAL(ImageUpdated()), this, SLOT(OnUpdate()));
		data->SetData(samples, oldData->GetBitPersample(), oldData->GetSampleRate());
		data->SetSize(oldData->GetSize());
		window = QRectF(oldData->GetWindow().x(), data->GetMaxWindow().y(), oldData->GetWindow().width(), data->GetMaxWindow().height());
		data->SetWindow(window);

		delete oldData;

		showWave = wave;
		return true;
	}
	return false;
}

bool WaveViewerWidget::IsWave() const
{
	return showWave;
}

void WaveViewerWidget::UpdateWindows()
{
	maxWindow = data->GetMaxWindow();
	minWindowSize = data->GetMinWindowSize();
	SetWindow(maxWindow.intersected(window));
}

void WaveViewerWidget::OnUpdate()
{
	update();
}

void WaveViewerWidget::OnCustomContextMenuRequested(const QPoint & pos)
{
	if ( hRullerRect.contains(pos) && menu_view )
		menu_view->exec(mapToGlobal(pos));
	else if ( menu_edit )
	{
		menu_edit->exec(mapToGlobal(pos));
	}
}

Button * WaveViewerWidget::SelectAndPressButton(QMouseEvent * event)
{
	Button * res = markingController->SelectAndPressButton(event);
	if ( res )
		return res;
	
	res = selectorController->SelectAndPressButton(event);
	if ( res )
		return res;

	return scrollController->SelectAndPressButton(event);
}

Button * WaveViewerWidget::GetButton(const QPoint & pos)
{
	Button * res = markingController->GetButton(pos);
	if ( res )
		return res;
	
	res = selectorController->GetButton(pos);
	if ( res )
		return res;

	return scrollController->GetButton(pos);
}

void WaveViewerWidget::OnMoveTimeout()
{
	if ( curAction == SelectActon &&  enableSelectSamples )
	{
		long temp = ToSelectedSample(Transform(curPos).x());
		SelectSamples(qMin(temp, start_select_sample),qMax(temp, start_select_sample), false);
	}

	QRectF rect = GetWindow();
	rect.moveCenter(rect.center() + offset);
	SetWindow(rect);
}
