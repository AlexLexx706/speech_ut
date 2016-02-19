#include "PipeClientPart.h"
#include <windows.h>
 
int main(void) 
{
	PipeClientPart client;
	if( client.InitConnection() )
	{
		const char * packet;
		int size;
		if ( client.ReadFromPipe(packet, size) )
		{
			client.WriteToPipe(packet, size);
			return 0;
		}
	}
	return 1;
}
