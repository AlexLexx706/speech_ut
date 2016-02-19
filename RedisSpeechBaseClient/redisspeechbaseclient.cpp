#include "redisspeechbaseclient.h"
#include "redisspeechbaseclientinternal/redisspeechbaseclientinternal.h"

RedisSpeechBaseClient::RedisSpeechBaseClient():c(new RedisSpeechBaseClientInternal())
{}

RedisSpeechBaseClient::~RedisSpeechBaseClient()
{
	delete c;
}

bool RedisSpeechBaseClient::Connect(const std::string & name, const std::string & port)
{
	return c->Connect(name, port);
}

bool RedisSpeechBaseClient::Disconnect()
{
	return c->Disconnect();
}

bool RedisSpeechBaseClient::IsConnected() const
{
	return c->IsConnected();
}

std::string RedisSpeechBaseClient::GetLastErrorDesc() const
{
	return c->GetLastErrorDesc();
}

unsigned int RedisSpeechBaseClient::GetSpeakerModelsCount()
{
	return c->GetModelsCount("speakermodel");
}

unsigned int RedisSpeechBaseClient::GetPhonemeModelCount()
{
	return c->GetModelsCount("phonememodel");
}

unsigned int RedisSpeechBaseClient::GetNModelCount()
{
	return c->GetModelsCount("nmodel");
}

unsigned int RedisSpeechBaseClient::GetLanguageCount()
{
	return c->GetModelsCount("language");
}

bool RedisSpeechBaseClient::GetAllSpeakerModels(std::list<RSBModel * > & models)
{
	return c->GetModelsByName("speakermodel",models);
}

bool RedisSpeechBaseClient::GetAllPhonemeModel(std::list<RSBModel * > & models)
{
	return c->GetModelsByName("phonememodel",models);
}

bool RedisSpeechBaseClient::GetAllNModel(std::list<RSBModel * > & models)
{
	return c->GetModelsByName("nmodelmodel",models);
}

bool RedisSpeechBaseClient::GetAllLanguage(std::list<RSBModel * > & models)
{
	return c->GetModelsByName("languagemodel",models);
}

bool RedisSpeechBaseClient::GetModelsByFilter(const std::string & className, const std::string & filter, std::list<RSBModel * > & models)
{
	return c->GetModelsByFilter(className, filter, models);
}

bool RedisSpeechBaseClient::GetModelsByName(const std::string & className, std::list<RSBModel * > & models)
{
	return c->GetModelsByName(className, models);
}

unsigned int RedisSpeechBaseClient::GetModelsCount(const std::string & className)
{
	return c->GetModelsCount(className);
}