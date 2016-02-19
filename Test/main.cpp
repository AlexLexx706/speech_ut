#include <iostream>
#include "../RedisSpeechBaseClient/redisspeechbaseclient.h"
#include <algorithm>

int main()
{
	std::cout << "test" << std::endl;
	int ccc;
	RedisSpeechBaseClient client;
	client.Connect("192.168.10.78", "6379");
	
	if ( client.IsConnected() )
	{
		std::cout << client.GetSpeakerModelsCount() << std::endl;

		std::list<RSBModel * >list;
		if ( client.GetModelsByFilter("speakermodel", "gender:male", list) )
		{
			for ( std::list<RSBModel * >::iterator iter = list.begin(); 
				iter != list.end(); iter++ )
			{
				RSBModel * model = *iter;
				unsigned char xxx[] = {1,0,3};
				std::vector<unsigned char> data(3);
				std::copy(xxx, xxx+3, data.begin());
				model->RPushList("test", data);
				model->GetListSize("test");
				std::vector< std::vector<unsigned char> > data2;
				model->GetFieldList("test",0,-1, data2);


				std::cout << "name: " << model->GetFieldString("name") << std::endl;
				std::cout << "gender: " << model->GetFieldString("gender") << std::endl;
				std::cout << "channel: " << model->GetFieldString("channel") << std::endl;
				std::cout << "mtype: " << model->GetFieldInt("mtype") << std::endl;
				std::cout << "mclass: " << model->GetFieldInt("mclass") << std::endl;
				std::cout << "mdata: " << model->GetFieldString("mdata") << std::endl;
				std::cout << "hash: " << model->GetFieldString("hash") << std::endl;
				std::cout << "comment: " << model->GetFieldString("comment") << std::endl;
				std::cout << "creator: " << model->GetFieldString("creator") << std::endl;
				std::cout << std::endl;
				//std::cin >> ccc;
			}
		}
	}


	std::cin >> ccc;
	return 0;
}