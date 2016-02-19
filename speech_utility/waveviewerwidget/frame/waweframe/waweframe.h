#ifndef WAWE_FRAME_A_H
#define WAWE_FRAME_A_H
#include "../dataframe/dataframe.h"
class TFourier;

//! Класс формирует рисунок.
class Frame:public  QObject
{
	Q_OBJECT
public:
	Frame();
	
	//! Установить окно.
	virtual void SetWindow(const QRectF & window);
	
	//!Окно.
	inline QRectF GetWindow() const {return window;}
	
	//! Установить размеры.
	virtual void SetSize(const QSize & size);

	//! Получить рисунок..
	inline QPixmap GetPixmap() const {return pixmap;}

	//! рисунок подготовлен.
	inline bool IsReady() const {return isReady;}
signals:
	void ImageUpdated();

protected:
	QPixmap pixmap;
	QRectF window;
	bool isReady;

protected slots:
	//! Обновить рисунок.
	virtual void Update() = 0;
};

//!Рисунок вертикальной линейки.
class VRullerFrame:public Frame
{
public:
	VRullerFrame(): cellWidth(60), showSamples(true){}

	virtual void SetWindow(const QRectF & window);

	void SetCelWidth(int _cellWidth);

	inline int GetCellWidth() const {return cellWidth; }

	void ShowSamples(bool samples);

	bool IsShowSamples() const { return showSamples;}

	inline QVector<int> GetMarks() const {return marks;}

protected:
	int cellWidth;
	bool showSamples;
	QVector<int> marks;

	virtual void Update();
};


//!Рисунок горизонтальной линейки.
class HRullerFrame:public VRullerFrame
{
public:
	HRullerFrame():bitpersample(8000){}

	virtual void SetWindow(const QRectF & window);

protected:
	int bitpersample;

	virtual void Update();
};

//! рисунок данных.
class DataFrame:public Frame
{
	Q_OBJECT
public:
	DataFrame();

	//! Установить данные
	bool SetData(const QByteArray & data, int _bitsPerSample, int _sampleRate);

	//! Получить данные.
	bool GetData(QByteArray & data, long startSample, long count) const;

	//! Удалить данные.
	bool RemoveData(long startSample, long count);

	//! Вставить данные.
	bool InsertData(const QByteArray & data, long startSample);

	//! Количество семплов в данных.
	inline int GetCount() const { return data.size()/(bitsPerSample>>3); }

	//! Частота дискретизации.
	inline int GetSampleRate() const { return sampleRate; }

	//! Бит на отсчёт.
	inline int GetBitPersample() const { return bitsPerSample; }

	inline QRectF GetMaxWindow() const { return maxWindow;}

	inline QSizeF GetMinWindowSize() const { return minWindowSize; }

signals:
	void DataChanged(long count);

protected:
	QByteArray data;
	int sampleRate;
	int bitsPerSample;
	QRectF maxWindow;
	QSizeF minWindowSize;

	//! Обновить границы.
	virtual void UpdateBorders(){};

	virtual void BeforeChange(){};
	
	virtual void AfterChange(){isReady = true;};

	virtual void CheckDataReady(){};
};


//! Асцилограмма данных.
class WaweFrame:public DataFrame
{
public:
	WaweFrame();

protected:
	//! Размер точек.
	QSize pointSize;

	virtual void UpdateBorders();
	virtual void Update();
	virtual void CheckDataReady(){isReady = GetCount();}
};


//! Спектрограмма данных.
class SpectorFrame:public DataFrame
{
	Q_OBJECT
public:
	SpectorFrame();

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




#endif // FRAME_A_H
