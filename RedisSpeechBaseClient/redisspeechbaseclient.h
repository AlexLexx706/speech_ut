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

//! Предоставляет интерфейс для доступа к моделям базы.
class REDISSPEECHBASECLIENT_EXPORT RSBModel
{
public:
	//! Получить значение поля int.
	int GetFieldInt(const std::string & name);

	//! Получить значение поля double.
	double GetFieldDouble(const std::string & name);

	//! Получить значение поля str::string.
	std::string GetFieldString(const std::string & name);

	//! Получить поле данных.
	std::vector<unsigned char> GetFieldData(const std::string & name);

	//! Получить список в заданных пределах.
	bool GetFieldList(const std::string & name, int start, int end, std::vector< std::vector<unsigned char> > & result);

	//! Получить размер листа.
	int GetListSize(const std::string & name);

	//! Добавить в лист.
	int RPushList(const std::string & name, std::vector<unsigned char> & data);

	//! Получить имя класса
	std::string GetClassName() const {return className;}

	//! Получить id обьекта.
	std::string GetUID() const {return uid;}

	//! Описание ошибки.
	std::string GetErrorDesc() const {return errorDesc;}

protected:
	friend RedisSpeechBaseClientInternal;

	//! Конструктор.
	RSBModel(RedisSpeechBaseClientInternal & _client, const std::string & _className, std::string & _uid):
	   client(_client), className(_className), uid(_uid)
	{}
	
	//! Ссылка на клиент.
	RedisSpeechBaseClientInternal & client;

	//! Имя класса модели.
	std::string className;

	//! uid обьекта.
	std::string uid;

	std::string errorDesc;

};

//! Клиент для работы с базой redis, где храняться звуковые модели.
class REDISSPEECHBASECLIENT_EXPORT RedisSpeechBaseClient
{
public:
////////////////////////////////////////////////////////////////////////////////
	//! Конструктор.
	RedisSpeechBaseClient();

	//! Деструктор.
	~RedisSpeechBaseClient();

	//! Подключиться к базе.
	bool Connect(const std::string & name, const std::string & port);

	//! Отключиться от базы.
	bool Disconnect();

	//! Проверка подключения.
	bool IsConnected() const;

	//! Получить описание ошибки.
	std::string GetLastErrorDesc() const;

	//! Получить список моделей по имени класса, например className = speakermodel.
	bool GetModelsByName(const std::string & className, std::list<RSBModel * > & models);

	//! Получить список моделей по фильтру, например className = speakermodel, filter = gender:male.
	bool GetModelsByFilter(const std::string & className, const std::string & filter, std::list<RSBModel * > & models);

	//! Получить количество моделей класса в базе.
	unsigned int GetModelsCount(const std::string & className);

	//! Получить количество обьектов SpeakerModels
	unsigned int GetSpeakerModelsCount();

	//! Получить количество обьектов PhonemeModel
	unsigned int GetPhonemeModelCount();

	//! Получить количество обьектов PhonemeModel
	unsigned int GetNModelCount();

	//! Получить количество обьектов Language
	unsigned int GetLanguageCount();


	//! Получить список SpeakerModels моделей.
	bool GetAllSpeakerModels(std::list<RSBModel * > & models);

	//! Получить список honemeModel моделей.
	bool GetAllPhonemeModel(std::list<RSBModel * > & models);

	//! Получить список NModel моделей.
	bool GetAllNModel(std::list<RSBModel * > & models);

	//! Получить список Language моделей.
	bool GetAllLanguage(std::list<RSBModel * > & models);

private:
	RedisSpeechBaseClientInternal * c;

};

#endif // REDISSPEECHBASECLIENT_H
