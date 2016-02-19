#include "phrasedeldiaog.h"
#include "ui_phrasedeldiaog.h"
#define ui (*((Ui::PhraseDelDiaogClass *)_ui))

PhraseDelDialog::PhraseDelDialog(QWidget *parent)
	: QDialog(parent)
{
	_ui = new Ui::PhraseDelDiaogClass;

	ui.setupUi(this);
	ui.pushButton_noMove->setFocus();
}

PhraseDelDialog::~PhraseDelDialog()
{
	delete (Ui::PhraseDelDiaogClass *)_ui;
}

void PhraseDelDialog::on_pushButton_moveLeft_clicked(bool checked)
{
	done(MoveLeft);
}

void PhraseDelDialog::on_pushButton_moveRight_clicked(bool checked)
{
	done(MoveRight);
}

void PhraseDelDialog::on_pushButton_noMove_clicked(bool checked)
{
	done(NoMove);
}

void PhraseDelDialog::on_pushButton_removeWithFrame_clicked(bool checked)
{
	 done(RemoveWithFrame);
}


void PhraseDelDialog::on_pushButton_cancel_clicked(bool checked)
{
	reject();
}