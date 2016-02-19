#ifndef REDISSPEECHBASECLIENT_H
#define REDISSPEECHBASECLIENT_H

#include <list>
#include <string>
#include <vector>

#ifdef REDISSPEECHBASECLIENT_LIB
# define REDISSPEECHBASECLIENT_EXPORT __declspec(dllexport)
#else
# define REDISSPEECHBASECLIENT_EXPORT __declspec(dllimport)
#endif

class RedisSpeechBaseClientInternal;

//! ������������� ��������� ��� ������� � ������� ����.
class REDISSPEECHBASECLIENT_EXPORT RSBModel
{
public:
	//! �������� �������� ���� int.
	int GetFieldInt(const std::string & name);

	//! �������� �������� ���� double.
	double GetFieldDouble(const std::string & name);

	//! �������� �������� ���� str::string.
	std::string GetFieldString(const std::string & name);

	//! �������� ���� ������.
	std::vector<unsigned char> GetFieldData(const std::string & name);

	//! �������� ������ � �������� ��������.
	bool GetFieldList(const std::string & name, int start, int end, std::vector< std::vector<unsigned char> > & result);

	//! �������� ������ �����.
	int GetListSize(const std::string & name);

	//! �������� � ����.
	int RPushList(const std::string & name, std::vector<unsigned char> & data);

	//! �������� ��� ������
	std::string GetClassName() const {return className;}

	//! �������� id �������.
	std::string GetUID() const {return uid;}

	//! �������� ������.
	std::string GetErrorDesc() const {return errorDesc;}

protected:
	friend RedisSpeechBaseClientInternal;

	//! �����������.
	RSBModel(RedisSpeechBaseClientInternal & _client, const std::string & _className, std::string & _uid):
	   client(_client), className(_className), uid(_uid)
	{}
	
	//! ������ �� ������.
	RedisSpeechBaseClientInternal & client;

	//! ��� ������ ������.
	std::string className;

	//! uid �������.
	std::string uid;

	std::string errorDesc;

};

//! ������ ��� ������ � ����� redis, ��� ��������� �������� ������.
class REDISSPEECHBASECLIENT_EXPORT RedisSpeechBaseClient
{
public:
////////////////////////////////////////////////////////////////////////////////
	//! �����������.
	RedisSpeechBaseClient();

	//! ����������.
	~RedisSpeechBaseClient();

	//! ������������ � ����.
	bool Connect(const std::string & name, const std::string & port);

	//! ����������� �� ����.
	bool Disconnect();

	//! �������� �����������.
	bool IsConnected() const;

	//! �������� �������� ������.
	std::string GetLastErrorDesc() const;

	//! �������� ������ ������� �� ����� ������, �������� className = speakermodel.
	bool GetModelsByName(const std::string & className, std::list<RSBModel * > & models);

	//! �������� ������ ������� �� �������, �������� className = speakermodel, filter = gender:male.
	bool GetModelsByFilter(const std::string & className, const std::string & filter, std::list<RSBModel * > & models);

	//! �������� ���������� ������� ������ � ����.
	unsigned int GetModelsCount(const std::string & className);

	//! �������� ���������� �������� SpeakerModels
	unsigned int GetSpeakerModelsCount();

	//! �������� ���������� �������� PhonemeModel
	unsigned int GetPhonemeModelCount();

	//! �������� ���������� �������� PhonemeModel
	unsigned int GetNModelCount();

	//! �������� ���������� �������� Language
	unsigned int GetLanguageCount();


	//! �������� ������ SpeakerModels �������.
	bool GetAllSpeakerModels(std::list<RSBModel * > & models);

	//! �������� ������ honemeModel �������.
	bool GetAllPhonemeModel(std::list<RSBModel * > & models);

	//! �������� ������ NModel �������.
	bool GetAllNModel(std::list<RSBModel * > & models);

	//! �������� ������ Language �������.
	bool GetAllLanguage(std::list<RSBModel * > & models);

private:
	RedisSpeechBaseClientInternal * c;

};

#endif // REDISSPEECHBASECLIENT_H
