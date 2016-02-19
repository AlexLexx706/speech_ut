#ifndef PHRASE_WIDGET_H
#define PHRASE_WIDGET_H

#include <QList>
#include <QRectF>
#include <QFrame>
#include "../../../speech_utility_global.h"

class QPainter;
class Phrase;
class PhraseController;
class QColor;
class PhrasesList;
class ExtLineEdit;

//! Описывает список разметок.
class SPEECH_UTILITY_EXPORT PhrasesListData:public QFrame
{
	Q_OBJECT
public:
	enum RemoveAction
	{
		MoveLeftBorder = 0,
		MoveRightBorder = 1,
		NoMoveBorder = 2
	};

	PhrasesListData(QWidget * parent, QList<Phrase *> & data = QList<Phrase *>());

	~PhrasesListData();

	//! Есть пересечение.
	bool IsIntersected() const { return intersected; }

	//! Есть пересечение.
	void SetIntersected(bool v);

	//! Установить список фраз;
	void SetPhrases(const QList<Phrase *> & data);

	//! список фраз.
	const QList<Phrase * > & GetPhrases() const { return phrases;}

	//! Получить максимум.
	long GetMax() const;

	//! Разрезать фразы.
	bool CutPhrases(long pos, bool copyText);

	//! Вставить фразу.
	bool InsertPhrase(Phrase * phrase, bool copyText);

	//! Удалить фразу.
	bool RemovePhrase(Phrase * phrase, RemoveAction acton);

	//! Текущая фраза.
	Phrase * GetCurPhrase() const { return curPhrase; }

	//! Установить текущую фразу.
	bool SetCurPhrase(Phrase * phrase);

	//! Установить цвет.
	void SetColor(const QColor & color);

	//! Цвет.
	QColor & GetColor() const { return *color;}

	//! Максимум изменился.
	void UpdateMax();

	//!
	void SetSelected(bool v);

	void SetButtonsVisible(bool show);

	//! Удалить фрагмент, при этом границы фраз после endSample смещаются на длинну endSample - startSample.
	bool RemoveFragment(long startSample, long endSample);

	/*** Вставить фрагмент, при этом граница фразы попавшая в startSample увеличивается на count,
	 *   а все фразы после startSample, смещаются на count.
	*/
	bool InsertFragment(long startSample, long count);

	//! Возвращает значения всех фраз в виде строки.
	const QString & GetString() const{return string;}

	//! Получить лист.
	PhrasesList * GetList() const {return list;}

	//! Искать фразу.
	Phrase * SearchPhrase(const QString & name, bool useCaseSensitive, bool useWholeWord, bool & loopFinished);

	//! Редактировать фразу.
	void EditPhraseName(Phrase * phrase);

	//! Получить список фраз с именем name.
	QList<Phrase *> GetPhrasesByName(const QString & name, bool caseSensitive) const;

	void SetEditDisable(bool disable){is_edit_disable = disable;};

	bool isEditDisable() const {return is_edit_disable;};

signals:
	void ListDataChanged();

	//! Выбрали другую фразу/слово или изменили границы.
	void CurrentPhraseChanged();

	//! Данные фразы изменились.
	void PhraseDataChanged();

	//! На фразе щёлкнули дважды.
	void PhraseDoubleClick();

	//! Прокрутка колёска.
	void ScaleHRuller(qreal hWindowCenter, int mouseDelta);

	void clicked();

protected:
	virtual void paintEvent(QPaintEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void keyReleaseEvent(QKeyEvent * event);
	virtual void wheelEvent(QWheelEvent * event);

private slots:
	void OnEditingFinished();
	void OnReturnPressed();
	void OnTextChanged(const QString & text);
private:
	friend class Phrase;

	enum Action 
	{
		NoAction,
		MoveLeft,
		MoveRight,
		MoveAll
	};

	PhraseController * controller;

	//! Список.
	QList<Phrase * > phrases;

	//! текущая фраза.
	Phrase * curPhrase;

	bool intersected;

	//! цвет.
	QColor * color;

	PhrasesList * list;

	bool selected;

	int selectedWidth;

	Action action;

	QPoint offset;

	ExtLineEdit * lineEdit;

	bool blockEdit;

	QString string;

	Phrase * startSearch;

	QPoint curPos;

	bool is_edit_disable;

	QRect GetPhraseRect(const Phrase *);

	//! в семплы из координат окна
	long InvTranslate(long);

	//! в координата окна из семплов
	long Translate(long);

	//! Найти фразу 
	Phrase * GetPhrase(const QPoint & pos);

	//! Получить действе которое можно выполнить над фразой.
	Action GetAction(const Phrase * phrase, const QPoint & pos, QPoint & offset);

	//! Обновить вид курсора
	void UpdateCursorShape(Action act);

	//! Данные изменились.
	void DataChanged();

	void UpdateString();

	//! Найти имя.
	int FindName(int start, const QString & name, bool UseCaseSensitive, bool useWholeWord);
};

#endif // PHRASE_WIDGET_H
