#include "spectorframe.h"
#include <QPainter>
#include <math.h>
#include <assert.h>
#include "fft/fft.h"
#include <QTime>

///////////////////////////////////////////////////////////////
SpectorFrame::SpectorFrame(QByteArray & _data):
	DataFrame(_data),
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

SpectorFrame::~SpectorFrame()
{
	thread.Stop();
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
				QString::fromLocal8Bit(!data.size() ? "Нет данных!!!" : "Данных недостаточно для отображения!!"));
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