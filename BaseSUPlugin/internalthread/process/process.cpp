#include "process.h"
#include <string.h>
#include <QByteArray>

#define ALEX_USE_QPROCESS

#ifdef ALEX_USE_QPROCESS
	#include <QProcess>
#else 
	#include <windows.h>
#endif


struct PrivateData
{
	#ifndef ALEX_USE_QPROCESS
		PROCESS_INFORMATION ProcessInfo;
		QString ErrorDesc;
	#else
		QProcess process;
	#endif

};

#define pData (((PrivateData *)pd))

Process::Process()
{
	pd = new PrivateData;
	#ifndef ALEX_USE_QPROCESS
		memset(&pData->ProcessInfo, 0, sizeof(pData->ProcessInfo));
	#endif
}

Process::~Process(void)
{
	kill();
	delete pData;
	pd = NULL;
}

bool Process::start(const QString & program, const QStringList & arguments)
{
	kill();
	#ifndef ALEX_USE_QPROCESS
		if ( program.isEmpty() )
		{
			pData->ErrorDesc = "Empty program name";
			return false;
		}

		QString cmdLine = "\"" + program + "\"";
		foreach(QString str, arguments)
			cmdLine += " \"" + str + "\"";

		//Запускаем процесс.
		STARTUPINFOA si;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);

		// Start the child process. 
		if ( CreateProcessA(	NULL,									// No module name (use command line)
								cmdLine.toLocal8Bit().data(),			// Command line
								NULL,           						// Process handle not inheritable
								NULL,           						// Thread handle not inheritable
								FALSE,          						// Set handle inheritance to FALSE
								0,              						// No creation flags
								NULL,           						// Use parent's environment block
								NULL,           						// Use parent's starting directory 
								&si,            						// Pointer to STARTUPINFO structure
								&pData->ProcessInfo ) )  						// Pointer to PROCESS_INFORMATION structure
		{
			return true;
		}
		else
			pData->ErrorDesc = QString("CreateProcess error: %1").arg(::GetLastError());
		return false;
	#else
		pData->process.start(program, arguments);
		return pData->process.waitForStarted();
	#endif
}

bool Process::kill()
{
	#ifndef ALEX_USE_QPROCESS
		if ( pData->ProcessInfo.hProcess )
		{
			::TerminateProcess(pData->ProcessInfo.hProcess, 0);
			WaitForSingleObject(pData->ProcessInfo.hProcess, INFINITE );

			CloseHandle( pData->ProcessInfo.hProcess );
			CloseHandle( pData->ProcessInfo.hThread );

			memset(&pData->ProcessInfo, 0, sizeof(pData->ProcessInfo));
			return true;
		}
		return false;
	#else
		pData->process.kill();
		return true;
	#endif
}

QString Process::error() const
{
	#ifndef ALEX_USE_QPROCESS
		return pData->ErrorDesc;
	#else
		return pData->process.error();
	#endif
}