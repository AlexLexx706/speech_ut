#include "fft.h"
//#include "common.h"

#define _USE_MATH_DEFINES
#include <math.h>

TFourier::TFourier(int windowWidth)
{
  numAnSamples = windowWidth;
  numDftSamples = numAnSamples/2;

  _sin  = new double[numAnSamples];
  _cos  = new double[numAnSamples];

  pwr	= new double[numDftSamples];

  for (int i=1; i<numAnSamples; i++)
  {
    _sin[i] = sin(M_PI/i);
    _cos[i] = cos(M_PI/i);
  }

  logZero = 20.0 * log10 ( 32767.5 * numDftSamples );
}

//---------------------------------------------------------------------------
TFourier::~TFourier()
{
  delete _cos;
  delete _sin;  
  delete pwr;
}

//---------------------------------------------------------------------------
void TFourier::FFT(double *BuffRe, double *BuffIm, int index)
{
double c,s,t1,t2,t3,t4,u1,u2,u3;
int i,j,p,l,Lb,M,M1,K;

  Lb = 2*numDftSamples;
  M = numDftSamples;
  M1 = 2*numDftSamples-1;

 while (Lb >=2)
  {
   l =  floor(Lb/2.0);
   u1 = 1;
   u2 = 0;
   t1 = M_PI/l;
   c = _cos[l];
   s = (-1)*index*_sin[l];
   for (j = 0; j<l; j++)
    { i = j;
     while (i < 2*numDftSamples)
      {
       p = i+l;
       t1 = BuffRe[i]+BuffRe[p];
       t2 = BuffIm[i]+BuffIm[p];
       t3 = BuffRe[i]-BuffRe[p];
       t4 = BuffIm[i]-BuffIm[p];
       BuffRe[p] = t3*u1-t4*u2;
       BuffIm[p] = t4*u1+t3*u2;
       BuffRe[i] =t1;
       BuffIm[i] =t2;
       i=i+Lb;
      }
     u3 = u1*c-u2*s;
     u2 = u2*c+u1*s;
     u1 = u3;
    }
   Lb = floor(Lb/2.0);
  }
  j = 0;
  for ( i=0; i<M1; i++)
   {
    if (i > j)
     {
      t1 = BuffRe[j];
      t2 = BuffIm[j];
      BuffRe[j] = BuffRe[i];
      BuffIm[j] = BuffIm[i];
      BuffRe[i] = t1;
      BuffIm[i] = t2;
     }
    K = M;
    while(j >= K)
     {
       j = j-K;
       K = floor(K/2.0);
     }
    j = j+K;
   }

}
//---------------------------------------------------------------------------
void TFourier::FFTLogPower( double *BuffRe, double *BuffIm, double *BuffLogPwr )
{
	rms = 0;

	for( int i=0; i<numDftSamples; i++)
	{
		pwr[i] = (double)BuffRe[i]*BuffRe[i] + (double)BuffIm[i]*BuffIm[i];
		rms += pwr[i];

		if ( pwr[i] >= 1.0 )
		{
			BuffLogPwr[i] = 10.0 * log10 ( pwr[i] ) - logZero;
			
			/*
			if ( BuffLogPwr[i] > 0 )
			{
				BuffLogPwr[i] = 0;
			}

			*/

			if ( BuffLogPwr[i] < -FULL_LOG_SCALE )
			{
				BuffLogPwr[i] = -FULL_LOG_SCALE;
			}
		}
		else
		{
			BuffLogPwr[i] = -FULL_LOG_SCALE;
		}
	}

	rms = sqrt ( rms / numDftSamples );
}

double TFourier::GetPwr( int index )
{
	return sqrt ( pwr[index] );
}

double TFourier::GetRMS ()
{
	return rms;
}

double TFourier::GetRMSExcludingFundamental ( int index1, int index2 )
{
	long double rms2 = 0;

	for( int i=0; i<numDftSamples; i++)
	{
		if ( (i<index1) || (i>index2) )
		{
			rms2 += pwr[i];
		}
	}

	rms2 = sqrt ( rms2 / (numDftSamples - (index2-index1) ) );

	return (double)rms2;
}
