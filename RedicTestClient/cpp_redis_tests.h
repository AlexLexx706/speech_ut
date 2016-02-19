//
//  Copyright (c) 2009-2010,  Radu Braniste 
//  All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef cpp_redis_tests
#define cpp_redis_tests


#include <iostream>

#include "cpp_redis.h"

#include "cpp_redis_util.h"

#include <string>

namespace cpp_redis
{

typedef StringRequest<ResultExtractor> StringRequestOne;
typedef GenericRequest<ResultExtractor> GenericRequestOne;
typedef ListRequest<ResultExtractor> ListRequestOne;
typedef SetRequest<ResultExtractor> SetRequestOne;
typedef HashRequest<Printer> HashRequestOne;

void test_set_get( StringRequestOne& sr)
{
	PAIRS p;
	add2pairs("one", "una", p);

	sr.set(p);
	//printResults(sr);

	KEYS k;
	add2keys("one", k);
	sr.get(k);
	//printResults(sr);
}

void test_bin_set_get( StringRequestOne& sr)
{
	PAIRS p;
	int j = 16;
	add2pairs("bin", (unsigned char*)&j, sizeof(int) , p);

	sr.set(p);
	//printResults(sr.getResults());

	KEYS k;
	add2keys("bin", k);
	sr.get(k);
	print<int>(sr().getResults()->results_[0]);
}

void test_m_set_get( StringRequestOne& sr)
{

	PAIRS s;
	add2pairs("unu", "una", s);
	add2pairs("doi", "doua", s);
	add2pairs("trei", "treia", s);
	add2pairs("trouble", "buble", s);

	sr.set(s);
	//printResults(sr);

	KEYS k;
	add2keys("unu", k);
	add2keys("doi", k);
	add2keys("trei", k);
	add2keys("trouble", k);

	sr.get(k);
	//printResults(sr);
}

void test_setIfNotSet( StringRequestOne& sr)
{

	PAIRS s;
	add2pairs("radu", "test", s);

	sr.setIfNotSet(s);
	printResults(sr);
}

void test_getset( StringRequestOne& sr)
{
	PAIR p = makePair("alic", "kache");

	sr.getSet(p);
	//sr.get("alic");
	printResults(sr);
	
	sr.substr("alic", 1, 3);
	printResults(sr);
	
	KEYS k;
	add2keys("alic", k);
	sr.get(k);
	printResults(sr);
}
void test_append( StringRequestOne& sr)
{
	PAIR p = makePair("alic", "append");

	sr.append(p);
	//printResults(sr.getResults());
	
	KEYS k;
	add2keys("alic", k);
	sr.get(k);
	printResults(sr);
}


void test_incr( StringRequestOne& sr)
{

	sr.incr("gutu", 5);
	printResults(sr);
	
	sr.decr("gutu", 2);
	printResults(sr);
}



void test_size( GenericRequestOne& gr)
{
	gr.size();
	printResults(gr);
}

void test_exists( GenericRequestOne& gr)
{
	gr.exists("trouble");
	printResults(gr);
	gr.exists("buble");
	printResults(gr);
}

void test_type( GenericRequestOne& gr)
{
	gr.type("trouble");
	printResults(gr);
}

void test_del( GenericRequestOne& gr)
{
	KEYS k;
	add2keys("trouble", k);
	gr.del(k);
	printResults(gr);
}

void test_setdb( GenericRequestOne& gr)
{
	gr.setdb(2);
	printResults(gr);
}

void test_move( GenericRequestOne& gr)
{
	gr.move("trei", 1);
	printResults(gr);
}

void test_flush( GenericRequestOne& gr)
{
	gr.flushAll();
	//printResults(gr.getResults());
}

void test_rpush( ListRequestOne& lr)
{
	int j = 17;
	PAIR p = makePair("lista", (unsigned char*)&j, sizeof(int));
	lr.push(ListRequestOne::RIGHT, p);
	//printResults(lr.getResults());
	lr.llen("lista");
	printResults(lr);
}
void test_lpush( ListRequestOne& lr)
{
	PAIR p = makePair("lista", "nnuu");
	lr.push(ListRequestOne::RIGHT, p);
	//printResults(lr.getResults());
	lr.llen("lista");
	printResults(lr);
}

void test_lrange( ListRequestOne& lr)
{
	lr.lrange("lista", 0, 2);;
	print<int>(lr().getResults()->results_[0]);
	print<std::string>(lr().getResults()->results_[1]);
	////printResults(lr.getResults());
}

void test_blpop( ListRequestOne& lr)
{
	KEYS k;
	add2keys("lista2", k);
	add2keys("lista4", k);
	lr.bpop(ListRequestOne::LEFT, k, 2);
	printResults(lr);
}
void test_lset( ListRequestOne& lr)
{
	PAIR p = makePair("lista", "ssss");
	lr.lset(p, 0);
	//printResults(lr.getResults());
	lr.lrange("lista", 0, 2);;
	print<std::string>(lr().getResults()->results_[0]);
	print<std::string>(lr().getResults()->results_[1]);
}


void publish_ex(const std::string& topic, const std::string& payload)
{
Requestor<> rq;
SetRequest<ResultExtractor> s(rq);
ListRequestOne l(rq);

	s.smembers(topic);
	Result::RESULTS_TYPE r = s().getResults()->results_;
	
//std::cout <<  topic << "PUB!" << r.size() <<  std::endl ;
	for ( Result::RESULTS_TYPE::const_iterator i = r.begin(); i != r.end(); ++i)
	{
		std::string s(i->begin(), i->end());
		//std::cout << s.c_str() << "!" << std::endl ;
		PAIR p = makePair(s, payload);
		l.push(ListRequestOne::RIGHT, p);
	} 
}

void subscribe_ex(const std::string& topic, const std::string& key)
{

Requestor<> rq;
SetRequest<ResultExtractor> s(rq);
ListRequestOne l(rq);

while(1)
{
	PAIR p = makePair(topic, key);
	s.sismember(p);
	//std::cout <<  s_.getResults().intResults_[0] << "SUB!" << std::endl ;
	if (!s().getResults()->intResults_[0])
	{
		s.sadd(p);
	}
	KEYS k;
	add2keys(key, k);
	l.bpop(ListRequestOne::LEFT, k, 20);
	
	printResults(l);
	}
}

void publish(const std::string& topic, const std::string& payload)
{
	Requestor<> r;
	PubSub<Printer> ps(r);
	PAIR p = makePair(topic, payload);
	ps.publish(p);
}

void subscribe(int argc, char* argv[], int start = 2)
{
	Requestor<SUBSCRIBER> r;
	PubSub<Printer> ps(r);
	KEYS k;
	for (int i = start; i < argc; ++i)
	{
		add2keys(argv[i], k);
	}
	ps.subscribe(k);
}

template <class REQUESTOR>
void pipeline( REQUESTOR& c)
{

std::cout << "test accumulate" << std::endl;
	StringRequestOne r(c);
  	GenericRequestOne g(c);
  
	Pipeline<REQUESTOR, Printer> a(c);
  	test_flush(g);
	test_set_get(r);
	test_m_set_get(r);
}

template <class REQUESTOR>
void stringreq(REQUESTOR& c)
{
std::cout << "test stringreq" << std::endl;
StringRequestOne r(c);

  	GenericRequestOne g(c);

  	test_flush(g);
	test_getset(r);
	
	test_append(r);
	
	test_incr(r);
	
	test_setIfNotSet(r);
}

template <class REQUESTOR>
void multi( REQUESTOR& c )
{
	StringRequestOne sr(c);

	RedisTransaction<NullResult> commit(c);
	
	sr.incr("kao1", 3);
	
	sr.decr("kao", 2);
	
	commit();
printResults(sr);
}

template <class REQUESTOR>
void list( REQUESTOR& c )
{
	ListRequestOne l(c);
	test_rpush(l);
	test_lpush(l);
	test_lrange(l);
	test_blpop(l);
	test_lset(l);
}


template <class REQUESTOR>
void hash( REQUESTOR& c )
{
	HashRequestOne h(c);
	PAIR p = makePair("unu", "unu_value");
	h.hset("hash", p);
	h.hget("hash", "unu");
	h.hexists("hash", "doi");
	h.hlen("hash");
	p = makePair("doi", "doi_value");
	h.hset("hash", p);
	h.hfields("hash");
	h.hvals("hash");
	PAIRS s;
	add2pairs("ali", "baba", s);
	add2pairs("trouble", "buble", s);
	h.hmset("hash", s);
	h.hincrby("hash", "int", -10);
	h.hgetall("hash");
	KEYS k;
	add2keys("ali", k);
	add2keys("unu", k);
	h.hmget("hash", k);
	h.setIfNotSet("hash", p);
	h.hdel("hash", "int");
}

template <class REQUESTOR>
void set( REQUESTOR& c )
{
	SetRequest<Printer> s(c);
	Sort<Printer> g(c);
	PAIR p = makePair("ro", "unu");
	s.sadd( p);
	p = makePair("ro", "doi");
	s.sadd( p);
	p = makePair("en", "one");
	s.sadd( p);
	p = makePair("en", "two");
	s.sadd( p);
	p = makePair("ro", "two");
	s.smove("en", p);
	p = makePair("en", "two");
	s.sadd( p);
	s.smembers("en");
	s.smembers("ro");
	KEYS k;
	add2keys("ro", k);
	add2keys("en", k);
	s.sinter(k);
	s.sdiff(k);
	s.sunion(k);
	s.scard("en");
	g.alpha();
	g.isAsc(true);
	g.limit (0, 3);
	g("ro");
}
template <class REQUESTOR>
void sortedset( REQUESTOR& r )
{
	SortedSet<Printer> s(r);
	PAIR p = makePair("cheia", "val");
	s.zadd(1.2, p);
	//s.zincrby(-1.2, p);
	s.zscore(p);

	PAIR p1= makePair("cheia", "val1");
	s.zadd(1.3, p1);
	s.zrank(p);
	s.zcard("cheia");

	s.zrange("cheia", 0, -1, true);


PAIR p2 = makePair("lulu", "lala");
	s.zadd(11, p2);
std::vector<double> v;
double d[] = {4, 20, 2.3334};
std::copy (d, d + 2, std::back_inserter(v));
KEYS ks;
add2keys("lulu", ks);
add2keys("cheia", ks);

s.zunion("baba", ks, &v, SortedSet<Printer>::AGG_MAX);
s.zrange("baba", 0, -1, true);
}


template <class REQUESTOR>
void control( REQUESTOR& r )
{
	Control<Printer> c(r);
	c.save();
	c.bgrewriteaof();
	c.info();
	c.lastsave();
	c.quit();
	//c.shutdown();
}

}


#endif
