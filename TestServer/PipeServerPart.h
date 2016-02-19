#pragma once
struct PipeServerPart_InternalData;

class PipeServerPart
{
public:
	PipeServerPart(void);
	~PipeServerPart(void);
	bool CreateClientProcess(const char * processPath);
	void RemoveClientProcess();

	bool WriteToPipe(const char * data, int dataSize);

	bool ReadFromPipe(const char * & out, int & readedSize);

	bool IsConnected() const;

	const char * LastErrorDesc() const;

private:
	PipeServerPart_InternalData * data;
};
