//
//  Copyright (c) 2009-2010,  Radu Braniste 
//  All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __cpp_redis_impl
#define __cpp_redis_impl


#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <boost/system/error_code.hpp>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::error;
using namespace boost::system;


namespace cpp_redis
{

static unsigned char CRLF[] = {'\r', '\n'};
typedef std::vector<unsigned char> BYTES;
typedef std::string KEY;
typedef std::vector<KEY> KEYS;
typedef std::vector<std::pair<KEY, BYTES> > PAIRS;
typedef PAIRS::value_type PAIR;
typedef std::pair<int, BYTES> INT_VALUE;


const char* nil = "nil";
BYTES NIL(nil, nil+3);

struct Result
{
	typedef  std::vector<BYTES> RESULTS_TYPE;
	typedef  std::vector<int> INT_RESULTS_TYPE;
	void clear()
	{
		err_.clear();
		status_.clear();
		results_.clear();
		intResults_.clear();
		resultCode_ = 0;
	}
	int resultCode_; //0: results_; 2: int ; 4: err_;  8: status_
	std::string err_;
	std::string status_;
	INT_RESULTS_TYPE intResults_;
	RESULTS_TYPE results_;
};


struct Connection : private boost::noncopyable
{
	Connection(const std::string& host, const std::string& port, bool isAlive = false) :
			socket_(io_service_),  isAlive_(isAlive), isAccumulation_(false)
	{
		init(host, port);
	}
	Connection(bool isAlive = false) : 
		socket_(io_service_),  isAlive_(isAlive), isAccumulation_(false)
	{
		init("127.0.0.1", "6379");
	}
	void setLiveness(bool b)
	{
		isAlive_ = b;
	}
	void accumulate()
	{
		isAccumulation_ = true;
		accumulator_.clear();
	}
	template <class T>
	void runAccumulation(T& t)
	{
		Guard g(this);
		request(accumulator_, t, true);
	}
	template <class T>
	void request( BYTES& request, T& t, bool fromAccumulation = false)
	{
		if (isAccumulation_)
		{
			std::copy(request.begin(), request.end(), std::back_inserter(accumulator_));
			return;
		}
		asio::write(socket_, asio::buffer(&request[0], request.size()));
		asio::streambuf response;
		do
		{
			if (!reply(response) && fromAccumulation)
				break;
			t(result_);
		}
		while (isAlive_);
	}
private:
	struct Guard
	{
		Guard(Connection* c) : c_(c)
		{
			c_->isAccumulation_ = false;
			c_->isAlive_  = true;
		}
		~Guard()
		{
			c_->accumulator_.clear();
			c_->isAlive_ = false;
		}
	private:
	Connection* c_;
	};
private:
	bool reply(asio::streambuf& response)
	{
		int kk = -1;
		result_.clear();
		do
		{
			std::string length;
			readBytes(response, length);
			if (length.empty())
				return false;

			switch (length[0])
			{
				case '*' : 
				{
					int l = boost::lexical_cast<int>(length.c_str()+1);
					kk = l;
					if (l == -1)
					{
						setNil();
					}
					break;
				}
				case '$' : 
				{
					int l = boost::lexical_cast<int>(length.c_str()+1);
					if (l > 0) 
					{
						readBytes(response, l);
					}
					else if (l == -1)
					{
						setNil();
					}	
					else
					{
						std::cout << "UNEXPECTED: " << l << std::endl;
					}
					break;
				}
				case ':' : 
				{
					int l = boost::lexical_cast<int>(length.c_str()+1);
					setIntResult(l);
					break;
				}
				case '+' : 
				{
					readStatusBytes(&length[0]+1);
					break;
				}
				case '-' : 
				{
					readErrorBytes(&length[0]+1);
					break;
				}	
				default:
					return false;	
			} 
		}while (--kk> -1);
		return true;
	}
	void setNil()
	{
		BYTES b;
		b.push_back('n');b.push_back('i');b.push_back('l');
		result_.results_.push_back(NIL);
	}
	void setIntResult(int i )
	{
		result_.intResults_ .push_back(i);
		result_.resultCode_ = 2;
	}
	void setIntResults(const char* c, size_t len )
	{
		BYTES b;
		std::copy(c, c + len, std::back_inserter(b));
	}
	void readBytes(asio::streambuf& response, int l)
	{
		asio::read_until(socket_, response, "\r\n");
		std::istreambuf_iterator<char> i(&response);
		std::istreambuf_iterator<char> eos;
		BYTES v;
		while (i != eos) 
		{
			char c = *i++; 
			if ((c != '\r') && (c != '\n'))
				v.push_back(c);
			if (v.size() == l)
				break;
		}
		result_.results_.push_back(v);
	}
	void readErrorBytes(const char* r )
	{
		result_.err_ = r;
		result_.resultCode_ = 4;
	}
	void readStatusBytes(const char* r )
	{	result_.status_ = r;
		result_.resultCode_ = 8;
	}

	void readBytes(asio::streambuf& response, std::string& length)
	{
		asio::read_until(socket_, response, "\r\n");
		std::istream response_stream(&response);
		response_stream >> length;
	}
private:
	void init(const std::string& host, const std::string& port)
	{
		using asio::ip::tcp;
		tcp::resolver resolver(io_service_);
		tcp::resolver::query query(tcp::v4(),host, port);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;
		
		boost::system::error_code error = asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			socket_.close();
			socket_.connect(*endpoint_iterator++, error);
		}
		if (error)
			throw boost::system::system_error(error);
	}
	
private:
	asio::io_service io_service_;
	asio::ip::tcp::socket socket_;
	Result result_;
private:
	bool isAlive_;
	bool isAccumulation_;
	BYTES accumulator_;
};

template <class T>
PAIR makePair(const T& key, const std::string& value)
{
	PAIR u;
	u.first = key;
	std::copy(value.begin(), value.end(), std::back_inserter(u.second));
	return u;
}
template <class T>
PAIR  makePair(const T& key, const char* value)
{
	PAIR u;
	u.first = key;
	std::copy(value,value+strlen(value), std::back_inserter(u.second));
	return u;
}
template <class T>
PAIR  makePair(const T& key, const unsigned char* value, size_t length)
{
	PAIR u;
	u.first = key;
	std::copy(value,value+length, std::back_inserter(u.second));
	return u;
}
template<class T>
void makePairs(PAIR& cmd, T& p)
{
	if (cmd.first.empty())
		return;
	p.push_back(cmd);
}
void makePairs(PAIR& cmd, std::map<KEY, BYTES>& p)
{
	if (cmd.first.empty())
		return;
	p[cmd.first] = cmd.second;
}
BYTES toBytes(const std::string& value)
{
	BYTES v;
	std::copy(value.begin(), value.end(), std::back_inserter(v));
	return v;
}

template <class PRINTER>
struct RequestImpl : private boost::noncopyable
{
	RequestImpl(Connection& c) : c_(c)
	{}
	template <class P>
	void operator()( const std::string& command,  P& cmds)
	{
		int k = getCommands(cmds);
	
		BYTES v;
		addNumber(k, v, '*');
		addValueWithSize(command, v); 
		
		for (typename P::iterator i = cmds.begin(); i != cmds.end(); ++i)
		{
			typename P::value_type& p = *i;
			addPair(p, v);
		}
		c_.request( v, p_);
	}
	const PRINTER& operator()() 
	{
		return p_;
	}
private:
template <class T>
	int getCommands(T& cmds)
	{
		return (int)cmds.size() + 1;
	}
	int getCommands(PAIRS& cmds)
	{
		return (int)cmds.size()*2 + 1;
	}
	void addNumber(int k, BYTES& v, char start = '$')
	{
		try
		{
			std::string n = boost::lexical_cast<std::string>(k);
			v.push_back(start);
			addValue(n, v);
		}
		catch(boost::bad_lexical_cast &)
		{
			return;
		}
	}
	template <class T>
	void addValue(T* x, size_t sz, BYTES& v)
	{
		std::copy(x, x+sz, std::back_inserter(v));
		std::copy(CRLF, CRLF+2, std::back_inserter(v));
	}
	template<class T>
	void addValue(T& s, BYTES& v)
	{
		addValue(&s[0], s.size(), v);
	}
	template<class T>
	void addValueWithSize(T& s, BYTES& v)
	{
		addNumber((int)s.size(), v);
		addValue(&s[0], s.size(), v);
	}
	void addPair(PAIR& p, BYTES& v)
	{
		addValueWithSize(p.first, v);
		addValueWithSize(p.second, v);
	}
	template <class T>
	void addPair(T& p, BYTES& v)
	{
		addValueWithSize(p, v);
	}
				  
private:
	Connection& c_;
	PRINTER p_;
};

void add2pairs(const std::string& key, const std::string& v, PAIRS& s)
{
	PAIR p = makePair(key, v);
	makePairs(p, s);
}

void add2pairs(const std::string& key, const unsigned char* v, size_t length, PAIRS& s)
{
	PAIR p = makePair(key, v, length);
	makePairs(p, s);
}
void add2keys(const std::string& key, KEYS& k)
{
	k.push_back(key);
}
void insertInFrontOfKeys(const std::string& key, KEYS& k)
{
	k.insert(k.begin(), key);
}

template <class T>
void req1key1id(const std::string& command, const std::string& key, int id, T& t)
{
	std::string n = boost::lexical_cast<std::string>(id);
	req2keys(command, key, n, t);
}
template <typename F, class T>
void req1key2id(const std::string& command, const std::string& key, F id1, F id2, T& t)
{
	std::string start = boost::lexical_cast<std::string>(id1);
	std::string stop = boost::lexical_cast<std::string>(id2);
	req3keys(command, key, start, stop, t);
}
template <class T>
void req1key2id1key(const std::string& command, const std::string& key, int id1, int id2,
		 const std::string& key1,  T& t)
{
	std::string start = boost::lexical_cast<std::string>(id1);
	std::string stop = boost::lexical_cast<std::string>(id2);
	req4keys(command, key, start, stop, key1, t);
}
template <class T>
void req2keys1id(const std::string& command, const std::string& key1, const std::string& key2, 
				int id, T& t)
{
	std::string n = boost::lexical_cast<std::string>(id);
	req3keys(command, key1, key2, n, t);
}
template <class T>
void req1id(const std::string& command, int id, T& t)
{
	std::string n = boost::lexical_cast<std::string>(id);
	req1key(command, n, t);
}
template <class T>
void req1key(const std::string& command, const std::string& key, T& t)
{
	KEYS k;
	add2keys(key, k);
	t( command, k);
}
template <class T>
void req2keys(const std::string& command, const std::string& key1, const std::string& key2, T& t)
{
	KEYS k;
	add2keys(key1, k);
	add2keys(key2, k);
	t( command, k);
}

template <class T>
void req3keys(const std::string& command, const std::string& key1, const std::string& key2, const std::string& key3, T& t)
{
	KEYS k;
	add2keys(key1, k);
	add2keys(key2, k);
	add2keys(key3, k);
	t( command, k);
}

template <class T>
void req4keys(const std::string& command, const std::string& key1, const std::string& key2, const std::string& key3, const std::string& key4, T& t)
{
	KEYS k;
	add2keys(key1, k);
	add2keys(key2, k);
	add2keys(key3, k);
	add2keys(key4, k);
	t( command, k);
}
template <class T>
void reqPair( const std::string& command, PAIR& p, T& t)
{
	PAIRS cmds;
	makePairs(p, cmds);
	t(command, cmds);
}
template <typename F, class T>
void reqKeyIdValue(const std::string& command, PAIR& p, F id, T& t)
{
	std::vector<BYTES> v;
	v.push_back(toBytes(p.first));
	std::string r = boost::lexical_cast<std::string>(id);
	v.push_back(toBytes(r));
	v.push_back(p.second);
	t( command, v);
}
template <class T>
void reqKeyPair( const std::string& command, const std::string& key, PAIR& p, T& t)
{
	std::vector<BYTES> v;
	v.push_back(toBytes(key));
	v.push_back(toBytes(p.first));
	v.push_back(p.second);
	t( command, v);
}
template <class T>
void reqKeyPairs( const std::string& command, const std::string& key, PAIRS& p, T& t)
{
	std::vector<BYTES> v;
	v.push_back(toBytes(key));
	for (PAIRS::iterator i = p.begin(); i != p.end(); ++i)
	{
		v.push_back(toBytes(i->first));
		v.push_back(i->second);
	}
	t( command, v);
}
template <class T>
void reqkeykeys(const std::string& command, const std::string& key, KEYS& keys, T& t)
{
	insertInFrontOfKeys(key, keys);
	t( command, keys);
}

}

#endif
