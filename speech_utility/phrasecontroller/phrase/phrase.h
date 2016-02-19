#ifndef PHRASE_H
#define PHRASE_H

#include <QPair>
#include <QRectF>
#include "../../speech_utility_global.h"

class Button;
class PhrasesListData;
class QColor;

//! ��������� �����.
class SPEECH_UTILITY_EXPORT Phrase
{
public:
	//! ����������� 1.
	Phrase(PhrasesListData & list, long _start, long _end, const QString & _name);

	//! ����������� 2.
	Phrase(const Phrase & phrase);

	~Phrase();

	//! ��������.
	Phrase & operator = (const Phrase & phrase);

	//! �������� ���������.
	bool operator == (const Phrase & phrase);

	//! ������ ����� � �������.
	inline long GetStart() const { return start;}

	//! ���������� ������.
	void SetStart(long v);

	void MoveStart(long v);

	//! ��������� ����� �����.
	inline long GetEnd() const { return end;}

	//! ���������� �����.
	void SetEnd(long v);

	//! ������.
	inline long GetLen() const { return end - start;}

	//! ���������� ������.
	void SetLen(long v);

	//! ��� �����.
	inline QString GetName() const { return name;}

	//! ���������� ���.
	void SetName(const QString & _name);

	//! ���������� ������� ����� � �������, ������� ������������.
	QPair<long, long> GetBorder();

	//! ���������� ������.
	void SetButton(Button * button, bool start);

	inline QPair< Button *, Button * > GetButtons() const {return QPair< Button *, Button * >(startButton, endButton);}

	//! ����.
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
