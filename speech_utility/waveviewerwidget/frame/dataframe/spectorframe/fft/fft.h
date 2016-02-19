#pragma once

#define FULL_LOG_SCALE		150.0

class TFourier
  {
    double *_cos;
    double *_sin;

	double *pwr;
	long double rms;

	double logZero;
	int numAnSamples;
	int numDftSamples;

  public:

    TFourier(int windowWidth = 1024);
    ~TFourier();

    void FFT (double *BuffRe, double *BuffIm, int index);
    void FFTLogPower (double *BuffRe, double *BuffIm, double *BuffLogPwr );

	double GetPwr ( int index );
	double GetRMS ();
	double GetRMSExcludingFundamental ( int index1, int index2 );
	inline int NumAnSamples() const {return numAnSamples;};
	inline int NumDftSamples() const {return numDftSamples;};
  };