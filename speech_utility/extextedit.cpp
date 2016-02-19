#include "extextedit.h"
#include <QKeyEvent>

ExTextEdit::ExTextEdit(QWidget *parent)
	: QTextEdit(parent)
{}

ExTextEdit::~ExTextEdit()
{}


void ExTextEdit::keyPressEvent(QKeyEvent * e)
{
	if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
		emit editingFinished();
	else
		QTextEdit::keyPressEvent(e);
}