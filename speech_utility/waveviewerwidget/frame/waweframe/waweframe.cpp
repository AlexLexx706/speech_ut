#include "frame.h"
#include <QPainter>
#include <math.h>
#include <assert.h>
#include "fft/fft.h"
#include <QColor>
#include <QTime>


///////////////////////////////////////////////////////////////
Frame::Frame():
	isReady(true)
{}

void Frame::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		window =_window;
		Update();
	}
}

void Frame::SetSize(const QSize & _size)
{
	if ( pixmap.size() != _size )
	{
		pixmap = QPixmap(_size);
		Update();
	}
}

//////////////////////////////////////
void VRullerFrame::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		if ( window.height() != _window.height() || window.top() != _window.top() )
		{
			window = _window;
			Update();
		}
		else 
			window = _window;
	}
}

void VRullerFrame::SetCelWidth(int _cellWidth)
{
	if (_cellWidth != cellWidth )
	{
		cellWidth = _cellWidth;
		Update();
	}
}

void VRullerFrame::ShowSamples(bool _samples)
{
	if ( showSamples != _samples )
	{
		showSamples = _samples;
		Update();
	}
}


void VRullerFrame::Update()
{
	if ( !pixmap.isNull() )
	{
		//Горизонтальные линии.
		QPainter painter(&pixmap);
		pixmap.fill(Qt::gray);

		qreal dy = pixmap.height()/window.height();

		int hCount = pixmap.height()/cellWidth;
		qreal hStep = window.height()/hCount;
		if ( hStep < 1.) hStep = 1.;
		qreal start = ((int)(window.y()/hStep)) * hStep;
		qreal hStart = start - window.y();
		hCount = hCount+2;

		painter.setPen(Qt::white);
		painter.drawLine(0, 0, 0, pixmap.height());

		marks.resize(hCount);
		for (int i = 0; i < hCount; i++ )
		{
			qreal temp = hStep * i;
			qreal y = pixmap.height() - (hStart + temp) * dy;
			marks[i] = y;
			painter.drawLine(0, y, 3, y);
			painter.drawText(QRect(6, y - painter.fontMetrics().height()/2, pixmap.width() - 6, painter.fontMetrics().height()),
								Qt::AlignLeft | Qt::AlignVCenter,
								QString::number(temp + start));
		}
		emit ImageUpdated();
	}
}

////////////////////////////////////////////////////////////////
void HRullerFrame::SetWindow(const QRectF & _window)
{
	if ( window != _window )
	{
		if ( window.width() != _window.width() || window.left() != _window.left() )
		{
			window = _window;
			Update();
		}
		else 
			window = _window;
	}
}

QString ConvertToString2(qreal t)
{
	qreal ip;
	qreal fp = modf(t, &ip);
	long sec = ip;
	QString res = QString("%1").arg(fp).mid(1);
	long s = sec % 60;
	sec = sec - s;
	long m = sec / 60;
	res = (m != 0 ? QString::number(m) + ":" : "") + QString::number(s) + res;
	return res;
}


void HRullerFrame::Update()
{
	if ( !pixmap.isNull() )
	{
		QPainter painter(&pixmap);
		pixmap.fill(Qt::gray);

		//Разметка.
		qreal dx = pixmap.width()/window.width();

		long wCount = pixmap.width()/cellWidth;
		qreal wStep = window.width()/wCount;
		if ( wStep  < 1. )wStep = 1.;

		qreal start = ((long)(window.left()/wStep)) * wStep;
		qreal wStart = start - window.left();
		wCount = window.width()/wStep + 1;

		painter.setPen(Qt::white);
		painter.drawLine(0, 0, pixmap.width(), 0);
		painter.setFont(QFont("Arial", 7));

		marks.resize(wCount);

		//семплы
		if ( showSamples )
		{
			for (int i = 0; i < wCount; i++ )
			{
				qreal temp = wStep * i;
				qreal x = (wStart + temp) * dx;
				marks[i] = x;
				painter.drawLine(x, 0, x, 2);
				painter.drawText(x, painter.fontMetrics().height(), QString("%1").arg((long)(start + temp)));
			}
		}
		else 
		{
			qreal k = 1./bitpersample;

			for (int i = 0; i < wCount; i++ )
			{
				qreal temp = wStep * i;
				qreal x = (wStart + temp) * dx;
				marks[i] = x;
				painter.drawLine(x, 0, x, 2);
				painter.drawText(x, painter.fontMetrics().height(),
									ConvertToString2((start + temp)*k));
			}
		}
		emit ImageUpdated();
	}
}



//////////////////////////////////////////////////////////////////////
DataFrame::DataFrame():
	sampleRate(8000),
	bitsPerSample(16)
{}



bool DataFrame::SetData(const QByteArray & _data, int _bitsPerSample, int _sampleRate)
{
	if ( _bitsPerSample == 16 )
	{
		BeforeChange();
		
		bitsPerSample = _bitsPerSample;
		sampleRate = _sampleRate;
		data = _data;
		CheckDataReady();
		UpdateBorders();

		AfterChange();
		Update();
		emit DataChanged(GetCount());
	}
	else 
		assert(false);
	return true;
}

bool DataFrame::GetData(QByteArray & out, long startSample, long count) const
{
	if ( startSample >= 0 && startSample + count <= GetCount() )
	{
		if ( bitsPerSample == 16 )
		{
			out = data.mid(startSample << 1, count << 1);
			return true;
		}
		else 
			assert(false);
	}
	return false;
}

bool DataFrame::RemoveData(long startSample, long count)
{
	if ( count && startSample >= 0 && startSample + count <= GetCount() )
	{
		if ( bitsPerSample == 16 )
		{
			BeforeChange();

			data.remove((startSample << 1), (count << 1));
			CheckDataReady();
			UpdateBorders();

			AfterChange();
			Update();
			emit DataChanged(GetCount());
			return true;
		}
		else 
			assert(false);
	}
	return false;
}

bool DataFrame::InsertData(const QByteArray & inData, long startSample)
{
	if ( inData.size() && startSample >= 0 && startSample <= GetCount() )
	{
		if ( bitsPerSample == 16 )
		{
			BeforeChange();

			data.insert(startSample << 1, inData);
			CheckDataReady();
			UpdateBorders();

			AfterChange();
			Update();
			emit DataChanged(GetCount());
			return true;
		}
		else 
		{
			assert(0);
		}
	}
	return false;
}


bool GetProjection(qreal aX1, qreal aX2, qreal bX1, qreal bX2, qreal & left, qreal & right)
{
	qreal aL = (aX2 - aX1)/2;
	qreal bL = (bX2 - bX1)/2;
	
	//есть проекция.
	if ( fabs((aX1 + aL) - (bX1 + bL)) < aL + bL )
	{
		//Гранцы проекции.
		if ( bX1 > aX1 && bX1 < aX2 )
			left = bX1;
		else 
			left = aX1;

		if ( bX2 > aX1 && bX2 < aX2 )
			right = bX2;
		else 
			right = aX2;
		return true;
	}
	return false;
}


void ConnectUnconnected(short maxA, short minA, short & maxB, short & minB)
{
	short aL = (maxA - minA)/2;
	short bL = (maxB - minB)/2;

	qreal dir = (minA + aL) - (minB + bL);

	if ( fabs(dir) > aL + bL )
	{
		if ( dir > 0 )
			maxB = minA;
		else
			minB = maxA;
	}
}



////////////////////////////////////////////////////////////////
WaweFrame::WaweFrame():pointSize(4,4)
{
	UpdateBorders();
}


void WaweFrame::UpdateBorders()
{
	minWindowSize = QSizeF(10,10);

	if ( bitsPerSample == 16 )
	{
		short minY = 0;
		short maxY = 0; 

		if ( int size = GetCount() )
		{
			short * pData = (short *)data.data();
			minY = pData[0];
			maxY = pData[0];

			for ( int i = 0; i < size; i++ )
			{
				if ( pData[i] > maxY )
					maxY = pData[i];
				else if ( pData[i] <  minY )
					minY = pData[i];
			}
		}

		maxWindow = QRectF(-1,
			(maxY - minY) < minWindowSize.height() ? -minWindowSize.height()/2 - 1 : minY - 1, 
			GetCount() < minWindowSize.width() ? minWindowSize.width() + 2 : GetCount() + 2,
			(maxY - minY) < minWindowSize.height() ? minWindowSize.height() + 2 : (maxY - minY) + 2);


	}
	else 
		assert(false);
}



void WaweFrame::Update()
{
	if ( isReady )
	{
		QPainter painter(&pixmap);
		pixmap.fill(Qt::black);
		painter.setPen(QColor(75, 243, 167));
		painter.setBrush(QBrush(Qt::green,Qt::SolidPattern));

		short * pData = (short *)data.data();
		int size = GetCount();
	
		//Шаг отображения точек графика.
		qreal left, right;

		if ( GetProjection(0, size, window.left(), window.right(), left, right) )
		{
			int startIndex = left;
			int endIndex =  right + 1;

			qreal dx = 1./window.width() * pixmap.width();
			qreal dy = 1./window.height() * pixmap.height();

			// Проверка отрисовки.
			if ( startIndex >= 0 && startIndex < size )
			{
				qreal timeStep = window.width()/pixmap.width();

				//Уменьшаем масштаб.
				if ( timeStep >= 2 )
				{
					qreal curTime = 0.;
					short oldMax = pData[startIndex];
					short oldMin = oldMax;
					int x = 0;

					//Ищим максимумы и минимумы и отрисовка.
					while ( true )
					{
						short max = pData[startIndex];
						short min = max;
						bool exit = false;

						while ( curTime < timeStep )
						{
							if ( pData[startIndex] > max )
								max = pData[startIndex];
							else if ( pData[startIndex] < min )
								min = pData[startIndex];

							startIndex++;
							curTime++;

							//! Выходим из цикла
							if ( startIndex >= size || startIndex > endIndex )
							{
								exit = true;
								break;
							}
						}
						ConnectUnconnected(oldMax, oldMin, max, min);

						//Рисуем линии.
						painter.drawLine(x, pixmap.height() - (min - window.top()) * dy,
										 x, pixmap.height() - (max - window.top()) * dy);
						curTime = curTime - timeStep;
						x++;

						oldMin = min;
						oldMax = max;

						if ( exit )
							break;
					}
				}
				else 
				{
					int time = startIndex;
					QPoint oldPoint((time - window.left()) * dx, pixmap.height() - (pData[startIndex] - window.top()) * dy);
					startIndex++;

					int nextX = (startIndex <= endIndex && startIndex < size) ? 
								((time + 1 - window.left()) * dx) : oldPoint.x();

					bool drawPoint = abs(nextX - oldPoint.x()) > pointSize.width();

					for (; startIndex <= endIndex && startIndex < size; startIndex++ )
					{
						time++;
						QPoint point( ((time - window.left()) * dx),
								pixmap.height() - ((pData[startIndex] - window.top()) * dy));
						painter.drawLine(oldPoint, point);

						//Рисуем точки
						if ( drawPoint )
							painter.drawRect(point.x() - (pointSize.width() >> 1), 
											point.y() - (pointSize.height() >> 1), 
											pointSize.width(),
											pointSize.height());
						oldPoint = point;
					}			
				}
			}
		}
	}
	else
	{
		QPainter painter(&pixmap);
		painter.setFont(QFont("courer", 20));
		pixmap.fill(Qt::gray);
		painter.drawText(0,0, pixmap.width(), pixmap.height(), 
			Qt::AlignHCenter | Qt::AlignVCenter,
			QString::fromLocal8Bit("Нет данных!!!"));
	}

	emit ImageUpdated();
}



///////////////////////////////////////////////////////////////
SpectorFrame::SpectorFrame():
	fourier(new TFourier(512)),
	thread(*this),
	gradientImage(1,256,QImage::Format_ARGB32),
	procent(0)
{
	isReady = false;
	UpdateBorders();
	connect(this, SIGNAL(DataIsReady()), this, SLOT(OnDataIsReady()));
	connect(this, SIGNAL(ProcessData(int)), this, SLOT(Update()));

	QList< QPair< qreal, QColor > > gData;
	gData.append(QPair< qreal, QColor >(0., QColor(20,1,50)));
	gData.append(QPair< qreal, QColor >(0.3, QColor(100, 2, 110)));
	gData.append(QPair< qreal, QColor >(0.5, QColor(250, 230, 80)));
	gData.append(QPair< qreal, QColor >(1., QColor(255, 255, 255)));

	SetGradient(gData);

}

void SpectorFrame::SetGradient(const QList< QPair< qreal, QColor > > & _gradient)
{
	if ( gradient != _gradient )
	{
		gradient = _gradient;

		QPainter painter(&gradientImage);
		QLinearGradient linearGrad(QPointF(0,0), QPointF(0, 256));
		for ( QList< QPair< qreal, QColor > >::iterator iter = gradient.begin(); 
				iter != gradient.end(); iter++ )
			linearGrad.setColorAt(iter->first, iter->second);
		painter.setPen(Qt::NoPen);
		painter.fillRect(QRect(0,0,1, 256),linearGrad);
		UpdateColorTable();
		Update();
	}
}

QColor SpectorFrame::GetGradientColor(int index)
{
	index = index % 256;
	index = 255 - index;

	return gradientImage.pixel(0,index);
}

void SpectorFrame::UpdateColorTable()
{
	//Установим палитру. 
	QVector<QRgb> colors;
	colors.resize(256);

	for ( int i = 0; i < 256; i++ )
		colors[i] = GetGradientColor(i).rgb();
	dataImage.setColorTable(colors);
}



void SpectorFrame::Update()
{
	if ( isReady && GetCount() )
	{
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		qreal k1 = fourier->NumDftSamples();
		qreal k2 = sampleRate/fourier->NumAnSamples();

		QRectF w(window.x()/k1, dataImage.height() - (window.top() + window.height())/k2, window.width()/k1, window.height()/k2);
		if (w.width() <= 0 )
			w.setWidth(1);
		if (w.height() <= 0 )
			w.setHeight(0);
		painter.drawImage(QRectF(0,0,pixmap.width(), pixmap.height()), dataImage, w);
	}
	else
	{
		QPainter painter(&pixmap);
		painter.setFont(QFont("courer", 20));
		pixmap.fill(Qt::gray);

		if ( thread.isRunning() )
		{
			painter.drawText(0,0, pixmap.width(), pixmap.height(), 
				Qt::AlignHCenter | Qt::AlignVCenter,
				QString::fromLocal8Bit("Данные обрабатываются: %1%").arg(procent));
		}
		else 
		{
			painter.drawText(0,0, pixmap.width(), pixmap.height(), 
				Qt::AlignHCenter | Qt::AlignVCenter,
				QString::fromLocal8Bit("Нет данных!!!"));
		}
	}

	emit ImageUpdated();
}

void SpectorFrame::UpdateBorders()
{
	minWindowSize = QSizeF(10,10);
	maxWindow = QRectF(-1, -1,
		GetCount() < minWindowSize.width() ? minWindowSize.width() + 2 : GetCount() + 2,
		sampleRate/2+1);

}

void SpectorFrame::BeforeChange()
{
	thread.Stop();
	procent = 0;
	isReady = false;
}

void SpectorFrame::AfterChange()
{
	thread.Start();
}


void SpectorFrame::CompileData(bool & exitFlag)
{
	emit ProcessData(0);

	if ( data.size() )
	{
		if ( bitsPerSample == 16 )
		{
			//Создадим картинку.
			double max = 32768 * fourier->NumDftSamples();
			short * internalData = (short *)data.data();
			dataImage = QImage(GetCount()/fourier->NumDftSamples(), fourier->NumDftSamples(), QImage::Format_Indexed8);
			dataImage.fill(GetGradientColor(255).rgb());
			UpdateColorTable();

			if ( exitFlag )
			{
				emit StopProcessData();
				return;
			}


			int index = 0;
			double * re = new double[fourier->NumAnSamples()];
			double * im = new double[fourier->NumAnSamples()];

			double min = -150;// минимум в дицебелах.

			if ( exitFlag )
			{
				emit StopProcessData();
				return;
			}

			QTime time = QTime::currentTime();
			int number = 0;

			while ( index + fourier->NumAnSamples() < GetCount() )
			{
				memset(im, 0, sizeof(double)* fourier->NumAnSamples());
				for ( int i = 0; i < fourier->NumAnSamples(); i++ )
				{
					re[i] = internalData[index+i];
				}
				fourier->FFT(re, im, 1);

				for( int i = 0; i < fourier->NumDftSamples(); i++ )
				{
					double value = 20 * log(sqrt(re[i] * re[i] + im[i] * im[i]) / max);
					if ( value  < min )
						value = min;
					if ( value > 0 )
						value = 0;
					dataImage.setPixel(number, dataImage.height() - i - 1, (value/min) * 255);
				}

				index = index + fourier->NumDftSamples();
				number++;

				if ( exitFlag )
					break;

				if ( time.msecsTo(QTime::currentTime()) > 500 )
				{
					procent = (index/(qreal)GetCount()) * 100;
					emit ProcessData(procent);
					time = QTime::currentTime();
				}
			}
			delete [] re;
			delete [] im;
		}
		else 
			assert(false);

		emit ProcessData(100);
		emit StopProcessData();
		if ( !exitFlag )
			emit DataIsReady();
	}
}
void SpectorFrame::OnDataIsReady()
{
	isReady = true;
	Update();
}