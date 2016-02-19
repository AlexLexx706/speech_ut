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

//! ���������� ����.
class SPEECH_UTILITY_EXPORT PhraseController:public QScrollArea
{
	Q_OBJECT
public:
	enum SearchType
	{
		SearchInCurrentList,
		SearchInAll
	};

	//! �����������.
	PhraseController(QWidget * parent, ButtonsController * buttonController = NULL);
	
	//! ����������.
	~PhraseController();

	//! �������� ����� ������ ����.
	PhrasesList *  AddList();

	//! ������� ������ ����.
	bool RemoveList(PhrasesList * list);

	//! ���������� ������� ������ ������ ����.
	PhrasesList * GetCurList() const { return curList; }
	
	PhrasesListData * GetCurPhraseListData() const;

	//! ���������� ������� ������ ������ ����.
	bool SetCurList(PhrasesList * list);

	//! ������ �����.
	Phrase * GetCurPhrase() const;

	//! ������ �����.
	bool SetCurPhrase(Phrase * phrase);

	void SetButtonController(ButtonsController * controller){buttonController = controller;}

	//! ���������� ������.
	ButtonsController * GetButtonController() const { return buttonController;};

	//! ������������ ���������� �������.
	long GetMax() const {return max;}

	//! ����
	inline const QRectF & GetWindow() const { return window;}

	void SetSamplerate(int samplerate);

	//! ��������� � ������� ����.
	qreal ToWindowUnit(long sample) const;

	//! ��������� � ������� ����.
	long ToSample(qreal time, bool forSelect) const;
	
	void SetPhrasesListMenu(QMenu * menu) { phrasesListMenu = menu;}

	QMenu * GetPhrasesListMenu() const {return phrasesListMenu;}

	//! ������� ��������, ��� ���� ������� ���� ����� endSample ��������� �� ������ endSample - startSample.
	void RemoveFragment(long startSample, long endSample);

	void InsertFragment(long startSample, long count);

	//! ������ ����� �� �����.
	Phrase * SearchPhrase(const QString & name, bool useCaseSensitive, bool useWholeWord, SearchType type);

	//! ������� ��.
	void RemoveAll();

	const QList<PhrasesList * > & GetLists() const {return lists;}

	//! 
	bool IsSavePhraseTextWE() const { return savePhraseTextWE; }

	//! ��������� ����� ����� ��� ��������������.
	void SavePhraseTextWE(bool v){ savePhraseTextWE = v; }

	void SetMarkingFont(const QFont & font);

	QFont GetMarkingFont() const { return font;}

public slots:
	//! ���������� ����.
	void SetWindow(const QRectF & rect);

	//! ������������ ����� �������.
	void SetMax(long max);

	//! 
	void OnCustomContextMenuRequested(const QPoint & pos);

	//! ������� � ������� ��������
	void MoveUp();

	//! ������� � ���� ������� ��������
	void MoveDown();

signals:
	//! ������� ������ ����.
	void CurrentListChanged();

	//! ������ ����� ����������.
	void ListDataChanged();

	//! ������� ������ ����� ��� �������� �������.
	void CurrentPhraseChanged();

	//! ������ ����� ����������.
	void PhraseDataChanged();

	//! �� ����� �������� ������.
	void PhraseDoubleClick();

	void DropFile(const QString & fileName);

	//! ��������� ������.
	void ScaleHRuller(qreal hWindowCenter, int mouseDelta);

protected:
	void dragEnterEvent(QDragEnterEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void dragLeaveEvent(QDragLeaveEvent *event);

	void dropEvent(QDropEvent *event);

private:
	ButtonsController * buttonController;

	//! �������� ����.
	QList<PhrasesList * > lists;

	//! ������� ������.
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
