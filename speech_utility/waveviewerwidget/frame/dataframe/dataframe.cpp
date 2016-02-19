#include "dataframe.h"
#include <QPainter>
#include <math.h>
#include <assert.h>


DataFrame::DataFrame(QByteArray & _data):
	data(_data),
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
