#include "PipeClientPart.h"
#include <windows.h>
#include <vector>
#include <sstream>

struct PipeClientPart_InternalData
{
	HANDLE hStdin, hStdout;
	std::vector<char> readBuffer;
	std::string lastErrorDesc;
	
	PipeClientPart_InternalData()
	{
		hStdin = INVALID_HANDLE_VALUE;
		hStdout = INVALID_HANDLE_VALUE;
	}
};


PipeClientPart::PipeClientPart(void):
	data(new PipeClientPart_InternalData())
{
}

PipeClientPart::~PipeClientPart(void)
{
	CloseConnection();
	delete data;
}


bool PipeClientPart::InitConnection()
{
	CloseConnection();
	data->hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
	data->hStdin = GetStdHandle(STD_INPUT_HANDLE);

	if ( data->hStdout == INVALID_HANDLE_VALUE || data->hStdin == INVALID_HANDLE_VALUE )
	{
		data->lastErrorDesc = "Invalid handles stdIn, stdOut !!!";
		return false;
	}
	SetStdHandle(STD_INPUT_HANDLE, INVALID_HANDLE_VALUE);
	SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE);
	SetStdHandle(STD_ERROR_HANDLE, INVALID_HANDLE_VALUE);
	return true;
}


void PipeClientPart::CloseConnection()
{
	::CloseHandle(data->hStdout);
	::CloseHandle(data->hStdin);
	data->hStdout = INVALID_HANDLE_VALUE;
	data->hStdin = INVALID_HANDLE_VALUE;
}

bool PipeClientPart::IsConnected() const
{
	return data->hStdout != INVALID_HANDLE_VALUE && data->hStdin  != INVALID_HANDLE_VALUE;
}

bool PipeClientPart::WriteToPipe(const char * inData, int dataSize)
{
	if ( !data->hStdout )
	{
		data->lastErrorDesc = "Pipe not Created";
		return false;
	}
	if ( !data )
	{
		data->lastErrorDesc = "data == NULL";
		return false;
	}

	if ( !dataSize )
	{
		data->lastErrorDesc = "dataSize = NULL";
		return false;
	}
	DWORD dwWritten;

	BOOL bSuccess = WriteFile(data->hStdout, &dataSize, sizeof(dataSize), &dwWritten, NULL);

	if ( !bSuccess )
	{
		if ( GetLastError() == ERROR_BROKEN_PIPE )
		{
			data->lastErrorDesc = "Pipe is closed";

			//! Закроем процесс клиента.
			CloseConnection();
		}
		else
		{
			std::stringstream ss; 
			ss << "Pipe WriteFile error:" << GetLastError();
			data->lastErrorDesc = ss.str();
		}
		return false;
	}

	bSuccess = WriteFile(data->hStdout, inData, dataSize, &dwWritten, NULL);

	if ( !bSuccess )
	{
		if ( GetLastError() == ERROR_BROKEN_PIPE )
		{
			data->lastErrorDesc ="Pipe is closed";

			//! Закроем процесс клиента.
			CloseConnection();
		}
		else
		{
			std::stringstream ss; 
			ss << "Pipe WriteFile error:" << GetLastError();
			data->lastErrorDesc = ss.str();
		}
		return false;
	}
	
	return true;
}

bool PipeClientPart::ReadFromPipe(const char * & out, int & readedSize)
{
	if ( !data->hStdin )
	{
		data->lastErrorDesc = "Pipe not Created";
		return false;
	}

	DWORD dwRead;
	int packetSize = 0;

	//Читаем данные из пайпа.
	BOOL bSuccess = ReadFile(data->hStdin, &packetSize, sizeof(int), &dwRead, NULL);

	if( !bSuccess )
	{
		if ( GetLastError() == ERROR_BROKEN_PIPE )
		{
			data->lastErrorDesc = "Pipe is closed";

			//! Закроем процесс клиента.
			CloseConnection();
		}
		else
		{
			std::stringstream ss; 
			ss << "Pipe WriteFile error:" << GetLastError();
			data->lastErrorDesc = ss.str();
		}
		return false;
	}

	data->readBuffer.resize(packetSize);
	readedSize = 0;
	DWORD readSize = packetSize;
	CHAR chBuf[1000];

	while ( readedSize < packetSize )
	{
		BOOL bSuccess = ReadFile(data->hStdin, &chBuf, readSize, &dwRead, NULL);

		if( !bSuccess )
		{
			if ( GetLastError() == ERROR_BROKEN_PIPE )
			{
				data->lastErrorDesc = "Pipe is closed";

				//! Закроем процесс клиента.
				CloseConnection();
			}
			else
			{
				std::stringstream ss; 
				ss << "Pipe WriteFile error:" << GetLastError();
				data->lastErrorDesc = ss.str();
			}
			return false;
		}
		else
		{
			memcpy(&data->readBuffer[readedSize], chBuf, dwRead);
			readedSize += dwRead;
			readSize -= readedSize;
		}
	}
	out = readedSize ? &data->readBuffer[0] : NULL;
	return true;
}

const char * PipeClientPart::LastErrorDesc() const
{
	return data->lastErrorDesc.c_str();
}