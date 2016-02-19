#ifndef REDISSPEECHBASECLIENTINTERNAL_H
#define REDISSPEECHBASECLIENTINTERNAL_H

#include <list>
#include <string>


class RSBModel;

class RedisSpeechBaseClientInternal
{
public:
	//! Конструктор.
	RedisSpeechBaseClientInternal(void);
	
	//! Деструктор.
	~RedisSpeechBaseClientInternal(void);
	
	//! Подключиться к базе.
	bool Connect(const std::string & name, const std::string & port);

	//! Отключиться от базы.
	bool Disconnect();

	//! Проверка подключения.
	bool IsConnected() const{return (bool)requestor;}

	//! Получить описание ошибки.
	std::string GetLastErrorDesc() const{return lastErrorDesc;}

	//! Получить список моделей по фильтру, например className = speakermodel, filter = gender:male.
	bool GetModelsByFilter(const std::string & className, const std::string & filter, std::list<RSBModel * > & models);

	//! Получить список моделей по имени класса, например className = speakermodel.
	bool GetModelsByName(const std::string & className, std::list<RSBModel * > & models);

	//! Получить количество моделей класса в базе.
	unsigned int GetModelsCount(const std::string & className);


private:
	friend class RSBModel;
	void * requestor;
	std::string lastErrorDesc;
};

#endif //REDISSPEECHBASECLIENTINTERNAL_H