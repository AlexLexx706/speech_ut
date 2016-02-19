#ifndef REDISSPEECHBASECLIENTINTERNAL_H
#define REDISSPEECHBASECLIENTINTERNAL_H

#include <list>
#include <string>


class RSBModel;

class RedisSpeechBaseClientInternal
{
public:
	//! �����������.
	RedisSpeechBaseClientInternal(void);
	
	//! ����������.
	~RedisSpeechBaseClientInternal(void);
	
	//! ������������ � ����.
	bool Connect(const std::string & name, const std::string & port);

	//! ����������� �� ����.
	bool Disconnect();

	//! �������� �����������.
	bool IsConnected() const{return (bool)requestor;}

	//! �������� �������� ������.
	std::string GetLastErrorDesc() const{return lastErrorDesc;}

	//! �������� ������ ������� �� �������, �������� className = speakermodel, filter = gender:male.
	bool GetModelsByFilter(const std::string & className, const std::string & filter, std::list<RSBModel * > & models);

	//! �������� ������ ������� �� ����� ������, �������� className = speakermodel.
	bool GetModelsByName(const std::string & className, std::list<RSBModel * > & models);

	//! �������� ���������� ������� ������ � ����.
	unsigned int GetModelsCount(const std::string & className);


private:
	friend class RSBModel;
	void * requestor;
	std::string lastErrorDesc;
};

#endif //REDISSPEECHBASECLIENTINTERNAL_H