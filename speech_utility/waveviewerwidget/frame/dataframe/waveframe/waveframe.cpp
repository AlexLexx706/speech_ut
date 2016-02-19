#include "waveframe.h"
#include <QPainter>
#include <math.h>
#include <assert.h>
#include <QColor>

////////////////////////////////////////////////////////////////


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









WaveFrame::WaveFrame(QByteArray & _data):
	DataFrame(_data),
	pointSize(4,4)
{
	UpdateBorders();
}

void WaveFrame::UpdateBorders()
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



void WaveFrame::Update()
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
