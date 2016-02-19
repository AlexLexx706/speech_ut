#include "internalautputdevice.h"
#include <iostream>


const char * GetErrorDesc(HRESULT hr)
{
	struct ErrorDesc
	{
		HRESULT res;
		const char * desc;
	};
	ErrorDesc desc[] = 
	{
		{DS_OK                      ,"DS_OK                   "},      
		{DS_NO_VIRTUALIZATION       ,"DS_NO_VIRTUALIZATION          "},
		{DSERR_ALLOCATED            ,"DSERR_ALLOCATED               "},
		{DSERR_CONTROLUNAVAIL       ,"DSERR_CONTROLUNAVAIL          "},
		{DSERR_INVALIDPARAM         ,"DSERR_INVALIDPARAM            "},
		{DSERR_INVALIDCALL          ,"DSERR_INVALIDCALL             "},
		{DSERR_GENERIC              ,"DSERR_GENERIC                 "},
		{DSERR_PRIOLEVELNEEDED      ,"DSERR_PRIOLEVELNEEDED         "},
		{DSERR_OUTOFMEMORY          ,"DSERR_OUTOFMEMORY             "},
		{DSERR_BADFORMAT            ,"DSERR_BADFORMAT               "},
		{DSERR_UNSUPPORTED          ,"DSERR_UNSUPPORTED             "},
		{DSERR_NODRIVER             ,"DSERR_NODRIVER                "},
		{DSERR_ALREADYINITIALIZED   ,"DSERR_ALREADYINITIALIZED      "},
		{DSERR_NOAGGREGATION        ,"DSERR_NOAGGREGATION           "},
		{DSERR_BUFFERLOST           ,"DSERR_BUFFERLOST              "},
		{DSERR_OTHERAPPHASPRIO      ,"DSERR_OTHERAPPHASPRIO         "},
		{DSERR_UNINITIALIZED        ,"DSERR_UNINITIALIZED           "},
		{DSERR_NOINTERFACE          ,"DSERR_NOINTERFACE             "},
		{DSERR_ACCESSDENIED         ,"DSERR_ACCESSDENIED            "},
		{DSERR_BUFFERTOOSMALL       ,"DSERR_BUFFERTOOSMALL          "},
		{DSERR_DS8_REQUIRED         ,"DSERR_DS8_REQUIRED            "},
		{DSERR_SENDLOOP             ,"DSERR_SENDLOOP                "},
		{DSERR_BADSENDBUFFERGUID    ,"DSERR_BADSENDBUFFERGUID       "},
		{DSERR_OBJECTNOTFOUND       ,"DSERR_OBJECTNOTFOUND          "},
		{DSERR_FXUNAVAILABLE        ,"DSERR_FXUNAVAILABLE           "},
	};

	for ( int i = 0; i < sizeof(desc)/sizeof(ErrorDesc);i++ )
	{
		if ( desc[i].res == hr)
			return desc[i].desc;
	}
	return "Unknown Error";
}


#define DXUT_ERR_ALEX(str,hr) std::cout << __FILE__ << ": " << __LINE__ << " " << str << " error:" << GetErrorDesc(hr) << std::endl;

InternalAutputDevice::InternalAutputDevice(const QAudioFormat & format):
				curFormat(format),
				curError(QAudio::NoError),
				curState(QAudio::IdleState),
				g_pSound(NULL)
{

	freq = curFormat.frequency();

	g_pSoundManager = new CSoundManager();
	if( NULL == g_pSoundManager )
	{
		curError = QAudio::OpenError;
		return;
	}

	HRESULT hr;
	wnd = CreateWindowA("STATIC", "DS", 0, 0,0,0,0,0,0,0,0);
	if (wnd == NULL)
	{
		wnd = GetDesktopWindow();
		if (wnd == NULL)
			wnd = ::GetForegroundWindow();
	}


	if( FAILED(hr = g_pSoundManager->Initialize(wnd, DSSCL_PRIORITY )) )
	{
		curError = QAudio::OpenError;
		DXUT_ERR_ALEX( "g_pSoundManager->Initialize", hr );
		return;
	}
    
	if( FAILED(hr = g_pSoundManager->SetPrimaryBufferFormat( 2, 22050, 16 ) ) )
	{
		curError = QAudio::OpenError;
		DXUT_ERR_ALEX( "g_pSoundManager->SetPrimaryBufferFormat", hr );
		return;
	}
	timer.setSingleShot(false);
	timer.setInterval(10);
	connect(&timer, SIGNAL(timeout()), this, SIGNAL(notify()));
	connect(&timer, SIGNAL(timeout()), this, SLOT(OnNotify()));
	curState = QAudio::StoppedState;
}

InternalAutputDevice::~InternalAutputDevice()
{
	stop();
	delete g_pSound;
	delete g_pSoundManager;
	//DestroyWindow(wnd);
}

qint64 InternalAutputDevice::processedUSecs() const 
{
	if ( g_pSound )
	{
		DWORD pos;
		HRESULT hr;
		if( SUCCEEDED(hr = g_pSound->GetPlayPos(pos) ) )
		{
			//std::cout << "pos: " << pos << std::endl;
			qreal res = qreal(pos)/(curFormat.sampleSize()/8 * curFormat.channels());
			res = res/freq;
			res = res * 1e6;
			return res;
		}
		else
		{
			DXUT_ERR_ALEX( "g_pSound->GetPlayPos", hr );
		}
	}
	return 0;
}

void InternalAutputDevice::resume()
{
	if ( g_pSound && curState != QAudio::ActiveState )
	{
		DWORD pos;
		HRESULT hr;
		if( SUCCEEDED(hr = g_pSound->GetPlayPos(pos)) )
		{
			if ( SUCCEEDED(hr = g_pSound->Play(pos, 0, 0, 0, freq)) )
			{
				timer.start();
				curState = QAudio::ActiveState;
				emit stateChanged(curState);
			}
			else
				DXUT_ERR_ALEX( "g_pSound->Play", hr );
		}
		else 
			DXUT_ERR_ALEX( "g_pSound->GetPlayPos", hr );
	}
}

void InternalAutputDevice::suspend()
{
	if ( g_pSound && curState != QAudio::SuspendedState )
	{
		HRESULT hr;
		if( SUCCEEDED(hr = g_pSound->Stop()) )
		{
			timer.stop();
			curState = QAudio::SuspendedState;
			emit stateChanged(curState);
		}
		else 
			DXUT_ERR_ALEX( "g_pSound->Stop", hr );
	}	
}

void InternalAutputDevice::start(const QByteArray & _data)
{
	stop();

	if ( _data.size() && curState != QAudio::ActiveState )
	{
		HRESULT hr;

		if( g_pSound )
		{
			g_pSound->Stop();
			g_pSound->Reset();
		}
		// Free any previous sound, and make a new one
		SAFE_DELETE( g_pSound );
		QByteArray data = _data;

		if ( data.size() )
		{
			WAVEFORMATEX format;
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.nChannels = curFormat.channels();
			format.nSamplesPerSec = curFormat.frequency();
			format.wBitsPerSample = curFormat.sampleSize();
			format.nBlockAlign = format.nChannels * format.wBitsPerSample/8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			HRESULT hr;
			if( SUCCEEDED(hr = g_pSoundManager->CreateFromMemory( &g_pSound, 
													(BYTE *)data.data(), 
													data.size() < DSBSIZE_MAX ? data.size() : DSBSIZE_MAX, 
													&format,
													DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS)) )
			{
				if( SUCCEEDED(hr = g_pSound->Play(0, 0, 0, 0, freq)) )
				{
					timer.start();
					curState = QAudio::ActiveState;
					emit stateChanged(curState);
					return;
				}
				else 
					DXUT_ERR_ALEX( "g_pSound->Play", hr );
			}
			else 
				DXUT_ERR_ALEX( "g_pSoundManager->CreateFromMemory", hr );
		}
	}
}
void InternalAutputDevice::stop()
{
	if ( g_pSound )
	{
		if ( curState != QAudio::StoppedState )
		{
			HRESULT hr;
			if ( SUCCEEDED(hr = g_pSound->Stop()) )
			{
				timer.stop();
				curState = QAudio::StoppedState;
				emit stateChanged(curState);
			}
			else
				DXUT_ERR_ALEX( "g_pSound->Stop", hr );
			
		}
	}
}

void InternalAutputDevice::reset()
{
	if ( g_pSound )
	{
		HRESULT hr;
		if ( FAILED(hr = g_pSound->Reset()) )
			DXUT_ERR_ALEX( "g_pSound->Stop", hr );
	}
}

quint32  InternalAutputDevice::GetFrequency()
{
	return freq;
}

void InternalAutputDevice::SetFrequency(quint32 f)
{
	freq = f;
}

quint32 InternalAutputDevice::GetCurPos()
{
	DWORD pos = 0;
	if ( g_pSound )
	{
		HRESULT hr;
		if ( FAILED(hr = g_pSound->GetPlayPos(pos)) )
			DXUT_ERR_ALEX( "g_pSound->GetPlayPos", hr );
	}
	return pos;
}

void InternalAutputDevice::SetCurPos(quint32 pos)
{
	if ( g_pSound )
	{
		HRESULT hr;
		if ( FAILED(hr = g_pSound->SetPlayPos(pos)) )
			DXUT_ERR_ALEX( "g_pSound->GetPlayPos", hr );
	}
}


void InternalAutputDevice::OnNotify()
{
	if ( g_pSound )
	{
		if ( !g_pSound->IsSoundPlaying() )
		{
			timer.stop();
			if ( curState != QAudio::StoppedState )
			{
				curState = QAudio::StoppedState;
				emit stateChanged(curState);
			}
		}
	}
}