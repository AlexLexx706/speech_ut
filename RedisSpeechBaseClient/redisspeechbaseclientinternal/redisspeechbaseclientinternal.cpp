#include "redisspeechbaseclientinternal.h"
#include <iostream>
#include "cpp_redis.h"
#include "cpp_redis_util.h"
#include "../redisspeechbaseclient.h"
#include <stdio.h>


using namespace cpp_redis;

typedef StringRequest<ResultExtractor> StringRequestOne;
typedef GenericRequest<ResultExtractor> GenericRequestOne;
typedef ListRequest<ResultExtractor> ListRequestOne;
typedef SetRequest<ResultExtractor> SetRequestOne;
typedef HashRequest<Printer> HashRequestOne;

#define REQ ((Requestor<> *)requestor)

#define ÑREQ ((Requestor<> *)client.requestor)


RedisSpeechBaseClientInternal::RedisSpeechBaseClientInternal(void):
	requestor(NULL),
	lastErrorDesc("No connection")
{
}

RedisSpeechBaseClientInternal::~RedisSpeechBaseClientInternal(void)
{
	Disconnect();
}


bool RedisSpeechBaseClientInternal::Connect(const std::string & name, const std::string & port)
{
	Disconnect();
	try
	{
		requestor = new Requestor<>(name, port);
		return true;
	}
	catch(std::exception & e)
	{
		lastErrorDesc = e.what();
		Disconnect();
	}
	return false;
}

bool RedisSpeechBaseClientInternal::Disconnect()
{
	if ( requestor )
	{
		delete requestor;
		requestor = NULL;
		return true;
	}
	lastErrorDesc = "No connecton";
	return false;
}

bool RedisSpeechBaseClientInternal::GetModelsByFilter(const std::string & modelName, const std::string & filter, std::list<RSBModel * > & models)
{
	if ( requestor )
	{
		SetRequestOne sr(*REQ);
		sr.smembers(modelName + ":" + filter);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 0 )
		{
			for ( Result::RESULTS_TYPE::const_iterator iter = r->results_.begin(); 
				iter != r->results_.end(); ++iter )
				models.push_back( new RSBModel(*this, modelName, std::string(iter->begin(), iter->end())) );
			return models.size();
		}
		else
			lastErrorDesc = r->err_;
	}
	else 
		lastErrorDesc = "No connection";
		
	return 0;
}


bool RedisSpeechBaseClientInternal::GetModelsByName(const std::string & className, std::list<RSBModel * > & models)
{
	if ( requestor )
	{
		SetRequestOne sr(*REQ);
		sr.smembers("submitted." + className);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 0 )
		{
			for ( Result::RESULTS_TYPE::const_iterator iter = r->results_.begin(); 
				iter != r->results_.end(); ++iter )
				models.push_back( new RSBModel(*this, className, std::string(iter->begin(), iter->end())) );
			return models.size();
		}
		else
			lastErrorDesc = r->err_;
	}
	else 
		lastErrorDesc = "No connection";
		
	return 0;
}

unsigned int RedisSpeechBaseClientInternal::GetModelsCount(const std::string & className)
{
	if ( requestor )
	{
		SetRequestOne sr(*REQ);
		sr.scard("submitted." + className);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 2 && r->intResults_.size()  )
			return r->intResults_[0];
		else
			lastErrorDesc = r->err_;
	}
	else 
		lastErrorDesc = "No connection";
	return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////
int RSBModel::GetFieldInt(const std::string & name)
{
	if ( ÑREQ )
	{
		StringRequestOne sr(*ÑREQ);
		KEYS k;
		add2keys(className + ":" + uid + ":" + name, k);
		sr.get(k);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 0 && r->results_.size() )
			return atoi(std::string(r->results_[0].begin(),r->results_[0].end()).data());
		else 
			errorDesc =  r->err_;

	}
	else 
		errorDesc = "No connection";
	return 0;
}

double RSBModel::GetFieldDouble(const std::string & name)
{
	if ( ÑREQ )
	{
		StringRequestOne sr(*ÑREQ);
		KEYS k;
		add2keys(className + ":" + uid + ":" + name, k);
		sr.get(k);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 0 && r->results_.size() )
			return atof(std::string(r->results_[0].begin(),r->results_[0].end()).data());
		else 
			errorDesc =  r->err_;
	}
	else 
		errorDesc = "No connection";

	return 0.;
}

std::string RSBModel::GetFieldString(const std::string & name)
{
	if ( ÑREQ )
	{
		StringRequestOne sr(*ÑREQ);
		KEYS k;
		add2keys(className + ":" + uid + ":" + name, k);
		sr.get(k);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 0 && r->results_.size() )
			return std::string(r->results_[0].begin(),r->results_[0].end()).data();
		else 
			errorDesc =  r->err_;
	}
	else 
		errorDesc = "No connection";
	return "";
}


std::vector<unsigned char> RSBModel::GetFieldData(const std::string & name)
{
	if ( ÑREQ )
	{
		StringRequestOne sr(*ÑREQ);
		KEYS k;
		add2keys(className + ":" + uid + ":" + name, k);
		sr.get(k);
		const Result* r = sr().getResults();

		if ( r->resultCode_ == 0 && r->results_.size() )
			return r->results_[0];
		else 
			errorDesc =  r->err_;
	}
	else 
		errorDesc = "No connection";
	return std::vector<unsigned char>();
}


bool RSBModel::GetFieldList(const std::string & name, int start, int end, std::vector< std::vector<unsigned char> > & result)
{
	if ( ÑREQ )
	{
		ListRequestOne lr(*ÑREQ);
		lr.lrange(className + ":" + uid + ":" + name, start, end);
		printResults(lr);
		const Result* r = lr().getResults();

		if ( r->resultCode_ == 0 && r->results_.size() )
		{
			result = r->results_;
			return true;
		}
		else 
			errorDesc = r->err_;
	}
	else 
		errorDesc = "No connection";
	return false;
}

int RSBModel::GetListSize(const std::string & name)
{
	if ( ÑREQ )
	{
		ListRequestOne lr(*ÑREQ);
		lr.llen(className + ":" + uid + ":" + name);
		printResults(lr);
		const Result* r = lr().getResults();

		if ( r->resultCode_ == 2 && r->intResults_.size() )
			return r->intResults_[0];
		else 
			errorDesc = r->err_;
	}
	else 
		errorDesc = "No connection";
	return 0;
}


int RSBModel::RPushList(const std::string & name, std::vector<unsigned char> & data)
{
	if ( ÑREQ )
	{
		ListRequestOne lr(*ÑREQ);
		PAIR p;	
		p.first = className + ":" + uid + ":" + name;	
		p.second = data;
		lr.push(ListRequestOne::RIGHT, p);
		printResults(lr);

		const Result* r = lr().getResults();

		if ( r->resultCode_ == 2 && r->intResults_.size() )
			return r->intResults_[0];
		else 
			errorDesc = r->err_;
	}
	else 
		errorDesc = "No connection";
	return 0;
}

