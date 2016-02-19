//
//  Copyright (c) 2009-2010,  Radu Braniste 
//  All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __cpp_redis
#define __cpp_redis

#include "cpp_redis_impl.h"

namespace cpp_redis
{

enum REQUESTOR_TYPE  {REQUESTOR = 0, SUBSCRIBER = 1};

template  <REQUESTOR_TYPE RQ = REQUESTOR>
struct Requestor : private boost::noncopyable
{
	Requestor(const std::string& host, const std::string& port) :
			c_(host, port, RQ)
	{}
	Requestor() : c_(RQ)
	{}
	void accumulate()
	{
		c_.accumulate();
	}
	template <class PRINTER>
	void runAccumulation(PRINTER& t)
	{
		c_.runAccumulation(t);
	}
	Connection& operator()()
	{
		return c_;
	}
private:
Connection c_;
};



template <class PRINTER>
struct GenericRequest : private boost::noncopyable
{
	template <class REQUESTOR>
	GenericRequest(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void exists( const std::string& key)
	{
		req1key( "EXISTS", key, impl_);
	}
	void del( KEYS& cmds)
	{
		impl_( "DEL", cmds);
	}
	void type(const std::string& key)
	{
		req1key( "TYPE", key, impl_);
	}
	void rename( const std::string& oldkey, const std::string& newkey)
	{
		req2keys("RENAME", oldkey, newkey, impl_);
	}
	void renameIfNotSet( const std::string& oldkey, const std::string& newkey)
	{
		req2keys("RENAMENX", oldkey, newkey, impl_);
	}
	void size()
	{
		KEYS k;
		impl_("DBSIZE", k);
	}
	void setdb(int id)
	{
		req1id( "SELECT", id, impl_);
	}
	void flushdb(int id)
	{
		req1id( "FLUSHDB", id, impl_);
	}
	void flushAll()
	{
		KEYS k;
		impl_("FLUSHALL", k);
	}
	void move(const std::string& key, int id)
	{
		req1key1id("MOVE", key, id, impl_);
	}
	void expire(const std::string& key, int timeout)
	{
		req1key1id("EXPIRE", key, timeout, impl_);
	}
	void expireAt(const std::string& key, int timeout)
	{
		req1key1id("EXPIREAT", key, timeout, impl_);
	}
	void ttl(const std::string& key)
	{
		req1key( "TTL", key, impl_);
	}
private:
	RequestImpl<PRINTER> impl_;
};




template <class PRINTER>
struct StringRequest : private boost::noncopyable
{
	template <class REQUESTOR>
	StringRequest(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void set( PAIRS& cmds)
	{
		impl_( "MSET", cmds);
	}
	void get( KEYS& cmds)
	{
		impl_( "MGET", cmds);
	}
	void setIfNotSet( PAIRS& cmds)
	{
		impl_( "MSETNX", cmds);
	}
	void setex( PAIR& p, int timeout)
	{
		reqKeyIdValue("SETEX", p, timeout, impl_);
	}
	void getSet( PAIR& cmd)
	{
		reqPair("GETSET", cmd, impl_);
	}
	void incr( const std::string& key, int increment = 1)
	{
		req1key1id("INCRBY", key, increment, impl_);
	}
	void decr( const std::string& key, int decrement = 1)
	{
		req1key1id("INCRBY", key, -decrement, impl_);
	}
	void substr( const std::string& key, int start, int end)
	{
		req1key2id( "SUBSTR", key, start, end, impl_);
	}
	void append( PAIR& cmd)
	{
		reqPair("APPEND", cmd, impl_);
	}
private:
	RequestImpl<PRINTER> impl_;
	
};

template <class PRINTER>
struct ListRequest
{	
	enum DIRECTION {LEFT, RIGHT};

	template <class REQUESTOR>
	ListRequest(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void push(DIRECTION d, PAIR& p)
	{
		const char* direction = d == RIGHT ? "RPUSH" : "LPUSH";
		reqPair(direction, p, impl_);
	}
	void llen( const std::string& key)
	{
		req1key( "LLEN", key, impl_);
	}
	void lrange( const std::string& key, int start, int end)
	{
		req1key2id( "LRANGE", key, start, end, impl_);
	}
	void pop( DIRECTION d, const std::string& key)
	{
		const char* direction = d == RIGHT ? "RPOP" : "LPOP";
		req1key( direction, key, impl_);
	}
	void bpop( DIRECTION d, KEYS& cmds, int timeout = 0)
	{
		std::string t = boost::lexical_cast<std::string>(timeout);
		add2keys(t, cmds);
		const char* direction = d == RIGHT ? "BRPOP" : "BLPOP";
		impl_( direction, cmds);
	}
	void ltrim( const std::string& key, int start, int end)
	{
		req1key2id( "LTRIM", key, start, end, impl_);
	}
	void lindex( const std::string& key, int id)
	{
		req1key1id( "LINDEX", key, id, impl_);
	}
	void lset( PAIR& p, int id)
	{
		reqKeyIdValue("LSET", p, id, impl_);
	}
	void lrem( PAIR& p, int id)
	{
		reqKeyIdValue("LREM", p, id, impl_);
	}
	void rpoplpush( const std::string& srckey, const std::string& dstKey)
	{
		req2keys("RPOPLPUSH", srckey, dstKey, impl_);
	}
private:
	RequestImpl<PRINTER> impl_;
};

template <class PRINTER>
struct SetRequest : private boost::noncopyable
{
	template <class REQUESTOR>
	SetRequest(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void sadd(PAIR& p)
	{
		reqPair("SADD", p, impl_);
	}
	void srem(PAIR& p)
	{
		reqPair("SREM", p, impl_);
	} 
	void spop(const std::string& key)
	{
		req1key("SPOP",key, impl_);
	}
	void smove(const std::string& key, PAIR& p)
	{
		reqKeyPair("SMOVE", key, p, impl_);
	}
	void sismember(PAIR& p)
	{
		reqPair("SISMEMBER", p, impl_);
	}
	void sinter( KEYS& cmds)
	{
		impl_( "SINTER", cmds);
	}
	void sunion( KEYS& cmds)
	{
		impl_( "SUNION", cmds);
	}
	void sdiff( KEYS& cmds)
	{
		impl_( "SDIFF", cmds);
	}
	void sinterstore( const std::string& key, KEYS& cmds)
	{
		reqkeykeys( "SINTERSTORE", key, cmds, impl_);
	}
	void sunionstore( const std::string& key, KEYS& cmds)
	{
		reqkeykeys( "SUNIONSTORE", key, cmds, impl_);
	}
	void sdiffstore( const std::string& key, KEYS& cmds)
	{
		reqkeykeys( "SDIFFSTORE", key, cmds, impl_);
	}
	void smembers(const std::string& key)
	{
		req1key("SMEMBERS",key, impl_);
	}
	void scard(const std::string& key)
	{
		req1key("SCARD",key, impl_);
	}
private:
	RequestImpl<PRINTER> impl_;
};

template <class PRINTER>
struct HashRequest : private boost::noncopyable
{
	template <class REQUESTOR>
	HashRequest(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void hset(const std::string& key, PAIR& p)
	{
		reqKeyPair("HSET", key, p, impl_);
	}
	void hmset(const std::string& key, PAIRS& p)
	{
		reqKeyPairs("HMSET", key, p, impl_);
	}
	void setIfNotSet(const std::string& key, PAIR& p)
	{
		reqKeyPair( "HSETNX", key, p, impl_);
	}
	void hget(const std::string& key, const std::string& field)
	{
		req2keys("HGET", key, field, impl_);
	}
	void hmget(const std::string& key, KEYS& fields)
	{
		reqkeykeys("HMGET", key,  fields, impl_);
	}
	void hdel(const std::string& key, const std::string& field)
	{
		req2keys("HDEL", key, field, impl_);
	}
	void hincrby(const std::string& key, const std::string& field, int incr)
	{
		req2keys1id("HINCRBY", key, field, incr, impl_);
	}
	void hexists(const std::string& key, const std::string& field)
	{
		req2keys("HEXISTS", key, field, impl_);
	}
	void hlen(const std::string& key)
	{
		req1key("HLEN", key, impl_);
	}
	void hfields(const std::string& key)
	{
		req1key("HKEYS", key, impl_);
	}
	void hvals(const std::string& key)
	{
		req1key("HVALS", key, impl_);
	}
	void hgetall(const std::string& key)
	{
		req1key("HGETALL", key, impl_);
	}
private:
	RequestImpl<PRINTER> impl_;
};

template <class PRINTER>
struct SortedSet : private boost::noncopyable
{
	template <class REQUESTOR>
	SortedSet(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void zadd( double score, PAIR& p)
	{
		reqKeyIdValue("ZADD", p, score, impl_);  
	}
	void zincrby( double score, PAIR& p)
	{
		reqKeyIdValue("ZINCRBY", p, score, impl_);  
	}
	void zrem( PAIR& p)
	{
		reqPair("ZREM", p, impl_);  
	}
	void zrank( PAIR& p)
	{
		reqPair("ZRANK", p, impl_);  
	}
	void zrevrank( PAIR& p)
	{
		reqPair("ZREVRANK", p, impl_);  
	}
	void zrange( const std::string& key, int start, int end, bool withscores = false)
	{
		withscores  ? req1key2id1key( "ZRANGE", key, start, end, "WITHSCORES",  impl_) :
				req1key2id( "ZRANGE", key, start, end, impl_);
	}
	void zreverange( const std::string& key, int start, int end, bool withscores = false)
	{
		withscores  ? req1key2id1key( "ZREVRANGE", key, start, end, "WITHSCORES",  impl_) :
				req1key2id( "ZREVRANGE", key, start, end, impl_);
	}
	void zremrangebyrank( const std::string& key, int start, int end)
	{
		req1key2id( "ZREMRANGEBYRANK", key, start, end, impl_);
	}
	void zremrangebyscore( const std::string& key, double min, double max)
	{
		req1key2id( "ZREMRANGEBYSCORE", key, min, max, impl_);
	}
	void zrangebyscore( const std::string& key, double min, double max, 
		bool withscores = false, 
		int start = -1, int end = -1, bool hasLimit = false )
	{
		KEYS k;
		append2id(min, max, k);
		if  (hasLimit)
		{
			append2id(start, end, k);
		}
		if  (withscores)
		{
			add2keys("WITHSCORES", k);
		}
		impl_( "ZRANGEBYSCORE", k);
	}
	void zcard( const std::string& key)
	{
		req1key("ZCARD", key, impl_);
	}
	void zscore( PAIR& p)
	{
		reqPair("ZSCORE", p, impl_);
	}
	enum AGGREGATE_METHOD  {AGG_NONE, AGG_SUM, AGG_MIN, AGG_MAX};
	void zunion( const std::string& dstkey, KEYS& k, std::vector<double>* weights = 0,
			AGGREGATE_METHOD am = AGG_NONE)
	{
		KEYS k1;
		unionImpl(dstkey, k, weights, am, k1);
		impl_( "ZUNION", k1);
	}
	void zinter( const std::string& dstkey, KEYS& k, std::vector<double>* weights = 0,
			AGGREGATE_METHOD am = AGG_NONE)
	{
		KEYS k1;
		unionImpl(dstkey, k, weights, am, k1);
		impl_( "ZINTER", k1);
	}

private:
	template <typename F>
	void append2id( F id1, F id2, KEYS& k)
	{
		std::string start = boost::lexical_cast<std::string>(id1);
		std::string stop = boost::lexical_cast<std::string>(id2);
		add2keys(start, k);
		add2keys(stop, k);
	}
	void unionImpl(  const std::string& dstkey, KEYS& k1, std::vector<double>* weights ,
			AGGREGATE_METHOD am , KEYS& k)
	{
		add2keys(dstkey, k);
		std::string str = boost::lexical_cast<std::string>(k1.size());
		add2keys(str, k);
		std::copy(k1.begin(), k1.end(), std::back_inserter(k));
		if(weights && !weights->empty()) 
		{
			add2keys("WEIGHTS", k);
			for (std::vector<double>::iterator i = weights->begin(); 
					i != weights->end(); ++i)
			{
				std::string tp = boost::lexical_cast<std::string>(*i);
				k.push_back(tp);
			}
		}
		switch (am)
		{
			case AGG_SUM:
			{
				addAggregate("SUM", k);
				break;
			}
			case AGG_MIN:
			{
				addAggregate("MIN", k);
				break;
			}
			case AGG_MAX:
			{
				addAggregate("MAX", k);
				break;
			}
			default:
			break;
		}

	}
private:
	static void addAggregate(const std::string& aggregate, KEYS& k)
	{
		add2keys("AGGREGATE", k);
		add2keys(aggregate, k);
	}
private:
	RequestImpl<PRINTER> impl_;
};

template <class PRINTER>
struct Control : private boost::noncopyable
{
	template <class REQUESTOR>
	Control(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void save()
	{
		KEYS k;
		impl_("SAVE", k);  
	}
	void bgsave()
	{
		KEYS k;
		impl_("BGSAVE", k);
	}
	void bgrewriteaof()
	{
		KEYS k;
		impl_("BGREWRITEAOF", k);
	}
	void lastsave( )
	{
		KEYS k;
		impl_("LASTSAVE", k);  
	}
	void shutdown( )
	{
		KEYS k;
		impl_("SHUTDOWN", k);  
	}
	void quit( )
	{
		KEYS k;
		impl_("QUIT", k);  
	}
	void auth(const std::string& password )
	{
		req1key("AUTH", password, impl_);  
	}
	void info( )
	{
		KEYS k;
		impl_("INFO", k);  
	}
	void slaveof(const std::string& host, const std::string& port)
	{
		req2keys("SLAVEOF", host, port, impl_);
	}
	void slaveof()
	{
		req2keys("SLAVEOF", "NO", "ONE", impl_);
	}
private:
	RequestImpl<PRINTER> impl_;
};

template <class PRINTER>
struct PubSub : private boost::noncopyable
{
	template <class REQUESTOR>
	PubSub(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void publish( PAIR& p)
	{
		reqPair("PUBLISH", p, impl_);  
	}
	void subscribe( KEYS& cmds)
	{
		impl_( "SUBSCRIBE", cmds);
	}
	void unsubscribe( KEYS& cmds)
	{
		impl_( "UNSUBSCRIBE", cmds);
	}
private:
	RequestImpl<PRINTER> impl_;
};

template <class PRINTER>
struct RedisTransaction : private boost::noncopyable
{
	template <class REQUESTOR>
	RedisTransaction(REQUESTOR& c) : impl_(c)
	{
		KEYS k;
		impl_("MULTI", k);
	}
	void operator()()
	{
		KEYS k;
		impl_("EXEC", k);
	}
	void discard()
	{
		KEYS k;
		impl_("DISCARD", k);
	}
private:
	RequestImpl<PRINTER> impl_;
};

	
template <class REQUESTOR, class P>
struct Pipeline : private boost::noncopyable
{
	Pipeline(REQUESTOR& c) : c_(c)
	{
		c_.accumulate();
	}
	~Pipeline()
	{
		P prn;
		c_.runAccumulation(prn);
	}
private:
	REQUESTOR& c_;
};

template <class PRINTER>
struct Sort : private boost::noncopyable
{
	template <class REQUESTOR>
	Sort(REQUESTOR& c) : impl_(c())
	{}
	const PRINTER& operator()()
	{
		return impl_();
	}
	void operator()(const std::string& key )
	{
		reqkeykeys("SORT", key, k_, impl_);  
	}
	void limit( int start, int  count)
	{
		add2keys("LIMIT", k_);
		std::string s = boost::lexical_cast<std::string>(start);
		add2keys(s, k_);
		std::string c = boost::lexical_cast<std::string>(count);
		add2keys(c, k_);
	}
	void isAsc( bool b)
	{
		b ? add2keys("ASC", k_) : add2keys("DESC", k_);
	}
	void alpha()
	{
		add2keys("ALPHA", k_);
	}
	void get(const std::string& pattern)
	{
		getByPattern("GET", pattern);
	}
	void get()
	{
		getByPattern("GET", "#");
	}
	void by(const std::string& pattern)
	{
		getByPattern("BY", pattern);
	}
	void store(const std::string& key)
	{
		getByPattern("STORE", key);
	}
private:
	void getByPattern(const std::string& cmd, const std::string& pattern)
	{
		add2keys(cmd, k_);
		add2keys(pattern, k_);
	}
private:
	RequestImpl<PRINTER> impl_;
	KEYS k_;
};

}

#endif
