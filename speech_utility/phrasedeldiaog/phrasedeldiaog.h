#ifndef PHRASEDELDIAOG_H
#define PHRASEDELDIAOG_H

#include <QDialog>
#include "../speech_utility_global.h"

class SPEECH_UTILITY_EXPORT PhraseDelDialog : public QDialog
{
	Q_OBJECT
public:
	enum ResultCode
	{
		MoveLeft = 1,
		MoveRight = 2,
		NoMove = 3,
		RemoveWithFrame = 4
	};

	PhraseDelDialog(QWidget *parent = 0);
	~PhraseDelDialog();

private:
	void *_ui;

private slots:
	void on_pushButton_moveLeft_clicked(bool checked);
	void on_pushButton_moveRight_clicked(bool checked);
	void on_pushButton_noMove_clicked(bool checked);
	void on_pushButton_cancel_clicked(bool checked);
	void on_pushButton_removeWithFrame_clicked(bool checked);
};

#endif // PHRASEDELDIAOG_H
