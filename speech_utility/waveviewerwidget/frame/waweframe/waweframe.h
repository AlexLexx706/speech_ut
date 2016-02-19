#ifndef WAWE_FRAME_A_H
#define WAWE_FRAME_A_H
#include "../dataframe/dataframe.h"
class TFourier;

//! ����� ��������� �������.
class Frame:public  QObject
{
	Q_OBJECT
public:
	Frame();
	
	//! ���������� ����.
	virtual void SetWindow(const QRectF & window);
	
	//!����.
	inline QRectF GetWindow() const {return window;}
	
	//! ���������� �������.
	virtual void SetSize(const QSize & size);

	//! �������� �������..
	inline QPixmap GetPixmap() const {return pixmap;}

	//! ������� �����������.
	inline bool IsReady() const {return isReady;}
signals:
	void ImageUpdated();

protected:
	QPixmap pixmap;
	QRectF window;
	bool isReady;

protected slots:
	//! �������� �������.
	virtual void Update() = 0;
};

//!������� ������������ �������.
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


//!������� �������������� �������.
class HRullerFrame:public VRullerFrame
{
public:
	HRullerFrame():bitpersample(8000){}

	virtual void SetWindow(const QRectF & window);

protected:
	int bitpersample;

	virtual void Update();
};

//! ������� ������.
class DataFrame:public Frame
{
	Q_OBJECT
public:
	DataFrame();

	//! ���������� ������
	bool SetData(const QByteArray & data, int _bitsPerSample, int _sampleRate);

	//! �������� ������.
	bool GetData(QByteArray & data, long startSample, long count) const;

	//! ������� ������.
	bool RemoveData(long startSample, long count);

	//! �������� ������.
	bool InsertData(const QByteArray & data, long startSample);

	//! ���������� ������� � ������.
	inline int GetCount() const { return data.size()/(bitsPerSample>>3); }

	//! ������� �������������.
	inline int GetSampleRate() const { return sampleRate; }

	//! ��� �� ������.
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

	//! �������� �������.
	virtual void UpdateBorders(){};

	virtual void BeforeChange(){};
	
	virtual void AfterChange(){isReady = true;};

	virtual void CheckDataReady(){};
};


//! ������������ ������.
class WaweFrame:public DataFrame
{
public:
	WaweFrame();

protected:
	//! ������ �����.
	QSize pointSize;

	virtual void UpdateBorders();
	virtual void Update();
	virtual void CheckDataReady(){isReady = GetCount();}
};


//! ������������� ������.
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
	
	//! ����� ������� �������������.
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

	//! ���� ��������� �� 0 - 255
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
