//
//  Copyright (c) 2009-2010,  Radu Braniste 
//  All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include "cpp_redis_tests.h"

using namespace cpp_redis;

int test_pubsub_ex(int argc, char* argv[])
{
	if (argc == 3)
	{
		publish_ex(argv[1], argv[2]);
		return 0;
	}
	else if (argc == 4)
	{
		subscribe_ex(argv[1], argv[2]);
	}
	else
	{
		std::cout << "BAD";
	}
	return 0;
}

int test_pubsub(int argc, char* argv[])
{
	if ((argc == 4) && (!strcmp(argv[1], "-p")) )
	{
		publish(argv[2], argv[3]);
		return 0;
	}
	else if ((argc > 3) && (!strcmp(argv[1], "-s")) )
	{
		subscribe(argc, argv);
	}
	else
	{
		std::cout << "BAD";
		
	}
	return 0;	
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
  try
  {
	//return test_pubsub(argc, argv); 

	Requestor<> c("192.168.10.78", "6379");
	StringRequestOne s(c);
	s.append(makePair("k1", "xxxx"));

	KEYS k;
	add2keys("k1", k);
	add2keys("k2", k);
	add2keys("k31", k);
	s.get(k);
	printResults(s);


	//pipeline(c);
	stringreq(c);
	//multi(c);
	//list(c);
	//hash(c);
	//set(c);
	//control(c);
	//sortedset(c);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
