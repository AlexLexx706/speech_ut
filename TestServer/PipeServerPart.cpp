#include "PipeServerPart.h"
#include <windows.h>
#include <vector>
#include <sstream>

struct PipeServerPart_InternalData
{
	HANDLE g_hChildStd_IN_Rd;
	HANDLE g_hChildStd_IN_Wr;
	HANDLE g_hChildStd_OUT_Rd;
	HANDLE g_hChildStd_OUT_Wr;
	PROCESS_INFORMATION piProcInfo;
	std::vector<char> readBuffer;
	std::string lastErrorDesc;


	PipeServerPart_InternalData()
	{
		g_hChildStd_IN_Rd = NULL;
		g_hChildStd_IN_Wr = NULL;
		g_hChildStd_OUT_Rd = NULL;
		g_hChildStd_OUT_Wr = NULL;
		memset(&piProcInfo, 0, sizeof(piProcInfo));
	}
};

PipeServerPart::PipeServerPart(void):
	data(new PipeServerPart_InternalData())
{
}

PipeServerPart::~PipeServerPart(void)
{
	RemoveClientProcess();
	delete data;
}
bool PipeServerPart::CreateClientProcess(const char * processPath)
{
	RemoveClientProcess();

	if( !processPath )
	{
		data->lastErrorDesc = "processPath empty";
		return false;
	}

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	if ( ! CreatePipe(&data->g_hChildStd_OUT_Rd, &data->g_hChildStd_OUT_Wr, &saAttr, 0) ) 
	{
		std::stringstream ss;
		ss << "StdoutRd CreatePipe error:" << GetLastError();
		data->lastErrorDesc = ss.str();
		return false;
	}

	if ( ! SetHandleInformation(data->g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
	{
		std::stringstream ss;
		ss << "Stdout SetHandleInformation error:" << GetLastError();
		data->lastErrorDesc = ss.str();
		return false;
	}

	if (! CreatePipe(&data->g_hChildStd_IN_Rd, &data->g_hChildStd_IN_Wr, &saAttr, 0)) 
	{
		std::stringstream ss;
		ss << "Stdin CreatePipe error:" << GetLastError();
		data->lastErrorDesc = ss.str();
		return false;
	}

	if ( ! SetHandleInformation(data->g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
	{
		std::stringstream ss;
		ss << "Stdin SetHandleInformation" << GetLastError();
		data->lastErrorDesc = ss.str();

		return false;
	}

	// Create the child process. 
	STARTUPINFOA siStartInfo;
	BOOL bSuccess = FALSE; 

	// Set up members of the PROCESS_INFORMATION structure. 
	ZeroMemory( &data->piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory( &siStartInfo, sizeof(siStartInfo) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = data->g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = data->g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = data->g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcessA(NULL, 
							(LPSTR)processPath, // command line 
							NULL,          // process security attributes 
							NULL,          // primary thread security attributes 
							TRUE,          // handles are inherited 
							0,             // creation flags 
							NULL,          // use parent's environment 
							NULL,          // use parent's current directory 
							&siStartInfo,  // STARTUPINFO pointer 
							&data->piProcInfo);// receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
	{
		std::stringstream ss;
		ss << "CreateProcess error:" << GetLastError();
		data->lastErrorDesc = ss.str();
		return false;
	}
	else
	{
		::CloseHandle(data->piProcInfo.hThread);	
		data->piProcInfo.hThread = NULL;
		data->piProcInfo.dwThreadId = 0;
		data->piProcInfo.dwProcessId = 0;
	}

	//Close pipes
	::CloseHandle(data->g_hChildStd_OUT_Wr);
	data->g_hChildStd_OUT_Wr = NULL;
	::CloseHandle(data->g_hChildStd_IN_Rd);
	data->g_hChildStd_IN_Rd = NULL;
	return true;
}

void PipeServerPart::RemoveClientProcess()
{
	//!Остановим процесс.
	if ( data->piProcInfo.hProcess != NULL )
	{
		::TerminateProcess(data->piProcInfo.hProcess, 1);
		::CloseHandle(data->piProcInfo.hProcess);
		data->piProcInfo.hProcess = NULL;
	}

	//Закроем пайпы
	if ( data->g_hChildStd_OUT_Rd != NULL )
	{
		::CloseHandle(data->g_hChildStd_OUT_Rd);
		data->g_hChildStd_OUT_Rd = NULL;
	}

	if ( data->g_hChildStd_IN_Wr != NULL )
	{
		::CloseHandle(data->g_hChildStd_IN_Wr);
		data->g_hChildStd_IN_Wr = NULL;
	}
}

bool PipeServerPart::WriteToPipe(const char * inData, int dataSize)
{
	if ( !data->g_hChildStd_IN_Wr )
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

	BOOL bSuccess = WriteFile(data->g_hChildStd_IN_Wr, &dataSize, sizeof(dataSize), &dwWritten, NULL);

	if ( !bSuccess )
	{
		if ( GetLastError() == ERROR_BROKEN_PIPE )
		{
			data->lastErrorDesc = "Pipe is closed";

			//! Закроем процесс клиента.
			RemoveClientProcess();
		}
		else
		{
			std::stringstream ss;
			ss << "Pipe WriteFile error:" << GetLastError();
			data->lastErrorDesc = ss.str();
		}
		return false;
	}

	bSuccess = WriteFile(data->g_hChildStd_IN_Wr, inData, dataSize, &dwWritten, NULL);

	if ( !bSuccess )
	{
		if ( GetLastError() == ERROR_BROKEN_PIPE )
		{
			data->lastErrorDesc = "Pipe is closed";

			//! Закроем процесс клиента.
			RemoveClientProcess();
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

bool PipeServerPart::ReadFromPipe(const char * & out, int & readedSize)
{
	if ( !data->g_hChildStd_OUT_Rd )
	{
		data->lastErrorDesc = "Pipe not Created";
		return false;
	}

	DWORD dwRead;
	int packetSize = 0;

	//Читаем данные из пайпа.
	BOOL bSuccess = ReadFile(data->g_hChildStd_OUT_Rd, &packetSize, sizeof(int), &dwRead, NULL);

	if( !bSuccess )
	{
		if ( GetLastError() == ERROR_BROKEN_PIPE )
		{
			data->lastErrorDesc = "Pipe is closed";

			//! Закроем процесс клиента.
			RemoveClientProcess();
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
		BOOL bSuccess = ReadFile(data->g_hChildStd_OUT_Rd, &chBuf, readSize, &dwRead, NULL);

		if( !bSuccess )
		{
			if ( GetLastError() == ERROR_BROKEN_PIPE )
			{
				data->lastErrorDesc = "Pipe is closed";

				//! Закроем процесс клиента.
				RemoveClientProcess();
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

bool PipeServerPart::IsConnected() const
{
	return data->g_hChildStd_IN_Wr && data->g_hChildStd_OUT_Rd;
}

const char * PipeServerPart::LastErrorDesc() const
{
	return data->lastErrorDesc.c_str();
}