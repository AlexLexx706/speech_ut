#ifndef PHRASE_CONTROLLER_H
#define PHRASE_CONTROLLER_H

#include <QScrollArea>
#include <QVector>
#include <QPair>
#include <QRectF>
#include "../speech_utility_global.h"

class PhrasesList;
class ButtonsController;
class Phrase;
class QVBoxLayout;
class QSpacerItem;
class QMenu;
class PhrasesListData;

//! Контроллер фраз.
class SPEECH_UTILITY_EXPORT PhraseController:public QScrollArea
{
	Q_OBJECT
public:
	enum SearchType
	{
		SearchInCurrentList,
		SearchInAll
	};

	//! Конструктор.
	PhraseController(QWidget * parent, ButtonsController * buttonController = NULL);
	
	//! Деструктор.
	~PhraseController();

	//! Добавить новый список фраз.
	PhrasesList *  AddList();

	//! Удалить список фраз.
	bool RemoveList(PhrasesList * list);

	//! Возвращает текущий индекс списка фраз.
	PhrasesList * GetCurList() const { return curList; }
	
	PhrasesListData * GetCurPhraseListData() const;

	//! Установить текущий индекс списка фраз.
	bool SetCurList(PhrasesList * list);

	//! Текущя фраза.
	Phrase * GetCurPhrase() const;

	//! Текущя фраза.
	bool SetCurPhrase(Phrase * phrase);

	void SetButtonController(ButtonsController * controller){buttonController = controller;}

	//! контроллер кнопок.
	ButtonsController * GetButtonController() const { return buttonController;};

	//! Максимальное количество семплов.
	long GetMax() const {return max;}

	//! окно
	inline const QRectF & GetWindow() const { return window;}

	void SetSamplerate(int samplerate);

	//! переводит в единицы окна.
	qreal ToWindowUnit(long sample) const;

	//! переводит в единицы окна.
	long ToSample(qreal time, bool forSelect) const;
	
	void SetPhrasesListMenu(QMenu * menu) { phrasesListMenu = menu;}

	QMenu * GetPhrasesListMenu() const {return phrasesListMenu;}

	//! Удалить фрагмент, при этом границы фраз после endSample смещаются на длинну endSample - startSample.
	void RemoveFragment(long startSample, long endSample);

	void InsertFragment(long startSample, long count);

	//! Искать фразу по имени.
	Phrase * SearchPhrase(const QString & name, bool useCaseSensitive, bool useWholeWord, SearchType type);

	//! Удалить всё.
	void RemoveAll();

	const QList<PhrasesList * > & GetLists() const {return lists;}

	//! 
	bool IsSavePhraseTextWE() const { return savePhraseTextWE; }

	//! Обновлять текст фразы при редактирование.
	void SavePhraseTextWE(bool v){ savePhraseTextWE = v; }

	void SetMarkingFont(const QFont & font);

	QFont GetMarkingFont() const { return font;}

public slots:
	//! Установить окно.
	void SetWindow(const QRectF & rect);

	//! максимальное колво семплов.
	void SetMax(long max);

	//! 
	void OnCustomContextMenuRequested(const QPoint & pos);

	//! Перенос ы текущей разметки
	void MoveUp();

	//! Перенос в вниз текущей разметки
	void MoveDown();

signals:
	//! Выбрали другой лист.
	void CurrentListChanged();

	//! Данные листа изменились.
	void ListDataChanged();

	//! Выбрали другую фразу или изменили границы.
	void CurrentPhraseChanged();

	//! Данные фразы изменились.
	void PhraseDataChanged();

	//! На фразе щёлкнули дважды.
	void PhraseDoubleClick();

	void DropFile(const QString & fileName);

	//! Прокрутка колёска.
	void ScaleHRuller(qreal hWindowCenter, int mouseDelta);

protected:
	void dragEnterEvent(QDragEnterEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void dragLeaveEvent(QDragLeaveEvent *event);

	void dropEvent(QDropEvent *event);

private:
	ButtonsController * buttonController;

	//! Разметка слов.
	QList<PhrasesList * > lists;

	//! Текущий индекс.
	PhrasesList * curList;

	PhrasesList * searchList;

	QVBoxLayout * vLayout;

	QRectF window;

	long max;

	int samplerate;

	QSpacerItem * vSpacer;

	QMenu * phrasesListMenu;

	QList<QColor> preferredColors;
	
	int preferredColorsIndex;

	bool savePhraseTextWE;

	QFont font;

};

#endif // PHRASE_CONTROLLER_H
