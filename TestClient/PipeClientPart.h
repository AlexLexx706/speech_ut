#pragma once

struct PipeClientPart_InternalData;
class PipeClientPart
{
public:
	PipeClientPart(void);
	~PipeClientPart(void);
	
	bool InitConnection();
	
	void CloseConnection();
	
	bool IsConnected() const;

	bool WriteToPipe(const char * data, int dataSize);

	bool ReadFromPipe(const char * & out, int & readedSize);

	const char * LastErrorDesc() const;
private:
	PipeClientPart_InternalData * data;

};
