#ifndef PHRASE_LIST_H
#define PHRASE_LIST_H

#include <QFrame>
#include <QList>
#include "../../speech_utility_global.h"

class PhraseController;
class Phrase;
class PhrasesListData;
class QLabel;
class QCheckBox;
class QHBoxLayout;

//! Описывает список разметок.
class SPEECH_UTILITY_EXPORT PhrasesList:public QFrame
{
	Q_OBJECT
public:
	PhrasesList(PhraseController & controller, QList<Phrase *> & data = QList<Phrase *>());

	~PhrasesList();

	PhrasesListData & GetData() const;

	void SetName(const QString & name);

	const QString & GetName() const {return name;};

	void SetFileName(const QString & fileName);

	const QString & GetFileName() const { return fileName;}

	void SetEnable(bool enable);

	bool IsEnable();

	PhraseController & GetController() const {return controller;}

signals:
	void CurrentPhraseChanged();

	void ListDataChanged();

private slots:
	void OnStateChanged(int state);

private:
	void * ui;

	PhraseController & controller;

	QString name;

	QString fileName;
};

#endif // PHRASE_LIST_H
