#ifndef EXTEXTEDIT_H
#define EXTEXTEDIT_H

#include <QTextEdit>

class ExTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	ExTextEdit(QWidget *parent);
	~ExTextEdit();
signals:
	void editingFinished();
protected:
	virtual void keyPressEvent(QKeyEvent * e);
};

#endif // EXTEXTEDIT_H
