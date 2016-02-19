#ifndef EXT_LINE_EDIT_H
#define EXT_LINE_EDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>


//Расширенный LineEdit;
class ExtLineEdit:public QLineEdit
{
	Q_OBJECT
public:
	ExtLineEdit(QWidget * widget = NULL):QLineEdit(widget){}
	~ExtLineEdit(){qDebug() << "~ExtLineEdit()\n";}

signals:
	void EditInterrupted();

protected:
	void keyPressEvent(QKeyEvent * event)
	{
		QLineEdit::keyPressEvent(event);

		if (event->key() == Qt::Key_Escape )
			emit EditInterrupted();
	}
};

#endif // EXT_LINE_EDIT_H
