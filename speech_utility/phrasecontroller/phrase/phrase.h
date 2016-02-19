#ifndef PHRASE_H
#define PHRASE_H

#include <QPair>
#include <QRectF>
#include "../../speech_utility_global.h"

class Button;
class PhrasesListData;
class QColor;

//! Описывает фразу.
class SPEECH_UTILITY_EXPORT Phrase
{
public:
	//! Конструктор 1.
	Phrase(PhrasesListData & list, long _start, long _end, const QString & _name);

	//! Конструктор 2.
	Phrase(const Phrase & phrase);

	~Phrase();

	//! Оператор.
	Phrase & operator = (const Phrase & phrase);

	//! Оператор сравнение.
	bool operator == (const Phrase & phrase);

	//! Начало фразы в семплах.
	inline long GetStart() const { return start;}

	//! Установить начало.
	void SetStart(long v);

	void MoveStart(long v);

	//! Последний семпл фразы.
	inline long GetEnd() const { return end;}

	//! Установить конец.
	void SetEnd(long v);

	//! Длинна.
	inline long GetLen() const { return end - start;}

	//! Установить длинну.
	void SetLen(long v);

	//! Имя фразы.
	inline QString GetName() const { return name;}

	//! Установить имя.
	void SetName(const QString & _name);

	//! Возвращает границы фразы в семплах, границы включительно.
	QPair<long, long> GetBorder();

	//! Установить кнопки.
	void SetButton(Button * button, bool start);

	inline QPair< Button *, Button * > GetButtons() const {return QPair< Button *, Button * >(startButton, endButton);}

	//! лист.
	inline PhrasesListData & GetListData() const { return list;}

	const QColor & GetColor() const;

private:
	long start;
	long end;
	QString name;
	PhrasesListData & list;
	Button * startButton;
	Button * endButton;
};
#endif // PHRASE_H
