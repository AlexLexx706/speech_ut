//
//  Copyright (c) 2009-2010,  Radu Braniste 
//  All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#ifndef cpp_redis_util
#define cpp_redis_util


#include <iostream>

#include "cpp_redis.h"

#include <string>

namespace cpp_redis
{

struct ResultExtractor
{
	ResultExtractor() : r_(0)
	{}
	void operator()(const Result& t)
	{
		r_ = &t;
	}
	const Result* getResults() const
	{
		return r_;
	}
private:
	const Result* r_; 
};


struct Printer
{
	Printer() 
	{}
	void operator()(const Result& t)
	{
		this->operator()(&t);
	}
	void operator()(const Result* t)
	{
		if (!t)
			return;
		switch (t->resultCode_)
		{
		case 4:
		std::cout << "err: " << t->err_ << std::endl;
		break;
		case 8:
		std::cout << "status: " << t->status_ << std::endl;
		break;
		case 2:
		//std::cout << "intResult: " << t.intResult_ << std::endl;
		{
			std::cout << "< ";
			for ( Result::INT_RESULTS_TYPE::const_iterator i = t->intResults_.begin(); i != t->intResults_.end(); ++i)
			{
				std::cout << *i << " " ;
			} 
			std::cout << ">" << std::endl;
		}
		//break;
		default:
		{
			if (t->results_.empty())
				return;
			std::cout << "[ ";
			for ( Result::RESULTS_TYPE::const_iterator i = t->results_.begin(); i != t->results_.end(); ++i)
			{
				std::string s(i->begin(), i->end());
				std::cout << s << " " ;
			} 
			std::cout << "]" << std::endl;
		}
		}
	}
};


struct NullResult
{
	void operator()(const Result& t)
	{}
	const Result* getResults() const
	{
		return 0;
	}
};

template <class T>
static void print ( const BYTES& v)
{
	T *k = 0;
	k = (T*)&v[0];
	std::cout <<  *k <<  std::endl;
}
template<>
 void print<std::string> ( const BYTES& v)
{
	std::string s(v.begin(), v.end());
	std::cout << s << std::endl;
}

template<class T>
void printResults(T& t)
{
	Printer prn;
	prn(t().getResults());
}


}
#endif

