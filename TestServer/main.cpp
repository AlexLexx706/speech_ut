#include "PipeServerPart.h"
#include <string>
#include <iostream>
int main()
{
	PipeServerPart server;
	
	if( server.CreateClientProcess("TestClient.exe") )
	{
		std::string str("Test Message!!!!!!!!!!!!!\nxxx\nyyy");
		
		if( server.WriteToPipe(str.c_str(), str.size()+1) )
		{
			const char * buffer;
			int bufferSize;
			if ( server.ReadFromPipe(buffer, bufferSize) )
			{
				std::cout << buffer;
			}
		}
	}
}