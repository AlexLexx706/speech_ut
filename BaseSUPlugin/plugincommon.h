#ifndef PLUGIN_COMMON_H
#define PLUGIN_COMMON_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QPair>
#include <QDataStream>

class QSettings;
class QMenu;
class QToolBar;


//! Данные разметки.
struct SUPPhraseData
{
	//!Значение в отсчётах
	qint32 Start;
	//!Значение в отсчётах
	qint32 End;	
	//! Имя фразы
	QString Name;
	
	//! Конструктор.
	SUPPhraseData(): Start(0), End(0){}

	//! Конструктор.
	SUPPhraseData(const SUPPhraseData & data)
	{
		Start = data.Start;
		End = data.End;
		Name = data.Name;
	}

	//! Конструктор.
	SUPPhraseData(qint32 _Start, qint32 _End, QString _Name):Start(_Start),End(_End),Name(_Name){}

	//! Оператор = .
	SUPPhraseData & operator = (const SUPPhraseData & data )
	{
		Start = data.Start;
		End = data.End;
		Name = data.Name;
		return *this;
	}
};

//! Входные данные для функции GetMarking
struct SUPInputData
{
	//! Бит на отсчёт.
	int BitsPerSample;
	
	//! Частота дискретизации.
	int SampleRate;

	//! Буффер сигнала.
	QByteArray Signal;

	//! Интервалы в семплах, для которых рассчитывать разметку.
	QList< QPair<qint32, qint32> > Intervals;

	//! Настройки плагина. В большинстве плагинов не используется, но если используется, то передаётся строка в формате JSON.
	QString PluginSettings;
};

//! Выходные данные для функции GetMarking
struct SUPOutData
{
	//! список разметки.
	QList<SUPPhraseData> Marking;
	
	//! Дополнительные данные описывающие результат(сейчас это JSON строка.).
	QString AdditionData;
};

//! Результат обработки файла, при пакетной обработки.
struct SUPFileResult
{
	QString FileName;
	QString ErrorDesc;
	bool Result;
};



//Интерфейс плагина для speech_utility.
class SUPluginInterface
{
public:

	//! Действия поддерживаемые плагином.
	enum ActionsType
	{
		//! Получать разметку.
		MakeMarking = 0x01,

		//! Пакетная обработка.
		BatchProcessing = 0x02,
	};

	//! Деструктор.
	virtual ~SUPluginInterface(){}

	//! Инициализация плагина.
	virtual bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar) = 0;

	//! Завершение работы с плагином
	virtual bool Free() = 0;

	//! Состояние плагина.
	virtual bool IsInit() const = 0;

	//! Получить имя плагина.
	virtual QString GetPluginName() const  = 0;

	//! Получить действия поддерживаемые плагином.
	virtual int GetActionsTypes() const  = 0;

	//! получить диалог настройки.
	virtual void OpenSettingsDialog() = 0;

	/** 
	 * Запустить создание разметки, не блокирующая функция, должна пораждать поток создания разметки.
	 * Результат получается в сигнале реализованном в плагине.
	*/
	virtual bool StartMakeMarking(int ID, const SUPInputData & inData) = 0;

	/** 
	 * Запустить пакетную обработку данных, не блокирующая функция, должна пораждать поток создания разметки.
	 * Результат получается в сигнале реализованном в плагине.
	*/
	virtual bool StartBatchProcessing(int ID, const QString & settings) = 0;
	
	//! Остановить пакетную обработку данных.
	virtual bool StopBatchProcessing(int ID) = 0;

	//! Получить описание последней ошибки.
	virtual QString GetLastErrorDesc() const = 0;

	//! Получить список результатов после пакетной обработки.
	virtual void GetBatchResult(QList<SUPFileResult> & result) = 0;

	//! Получить настройки плагина.
	virtual QString GetPluginSettings() const = 0;

};

Q_DECLARE_INTERFACE(SUPluginInterface, "{05AF5C94-26E8-4410-B51C-3BE0E05EB7CD}");

#endif // PLUGIN_COMMON_H
