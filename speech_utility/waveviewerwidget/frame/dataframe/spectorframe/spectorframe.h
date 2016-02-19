#ifndef SPECTOR_FRAME_A_H
#define SPECTOR_FRAME_A_H

#include "../dataframe.h"
#include <QThread>
#include <QList>
#include <QPair>


class TFourier;


//! Спектрограмма данных.
class SpectorFrame:public DataFrame
{
	Q_OBJECT
public:
	SpectorFrame(QByteArray & data);
	~SpectorFrame();
	void SpectorFrame::SetGradient(const QList< QPair< qreal, QColor > > & _gradient);

signals:
	void ProcessData(int procent);
	void StopProcessData();
	void DataIsReady();

protected:
	
	//! Поток расчёта спектрограммы.
	class CompileDataThread:public QThread
	{
	public:
		CompileDataThread(SpectorFrame & _frame):
			frame(_frame),
			exitFlag(false)
		{}

		void Start()
		{
			Stop();
			exitFlag = false;
			start();
		}

		void Stop()
		{
			exitFlag = true;
			wait();
		}
	protected:
		virtual void run ()
		{
			frame.CompileData(exitFlag);
		}

	private:
		SpectorFrame & frame;
		bool exitFlag;
	};

	QImage dataImage;
	TFourier * fourier;
	CompileDataThread thread;
	QImage gradientImage;
	QList< QPair< qreal, QColor > > gradient;
	int procent;

	//! Цвет градиента от 0 - 255
	QColor GetGradientColor(int index);
	void UpdateColorTable();
	virtual void Update();
	virtual void UpdateBorders();
	virtual void BeforeChange();
	virtual void AfterChange();
	virtual void CompileData(bool & exitFlag);

private slots:
	void OnDataIsReady();

};




#endif // SPECTOR_FRAME_A_H
