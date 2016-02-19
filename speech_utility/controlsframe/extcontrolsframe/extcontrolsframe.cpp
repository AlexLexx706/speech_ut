#include "extcontrolsframe.h"
#include "ui_extcontrolsframe.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QFontDialog>

#define ui (*((Ui::ExtControlsFrame *)_ui))

ExtControlsFrame::ExtControlsFrame(QWidget *parent):
	ControlsFrame(parent),
		_ui( new Ui::ExtControlsFrame)
{
	ui.setupUi(this);

	connect(ui.pushButton_moveUp, SIGNAL(clicked()), 
		this, SIGNAL(MoveMarkingUp()));

	connect(ui.pushButton_moveDown, SIGNAL(clicked()), 
		this, SIGNAL(MoveMarkingDown()));

	connect(ui.pushButton_pause, SIGNAL(clicked()), this, SIGNAL(PauseClicked()));
}

ExtControlsFrame::~ExtControlsFrame()
{
	delete (Ui::ExtControlsFrame *)_ui;
}

long ExtControlsFrame::GetSelectedStart()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_start->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_start->text());
}

long ExtControlsFrame::GetSelectedEnd()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_end->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_end->text());
}


long ExtControlsFrame::GetPlayPos()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_playPos->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_playPos->text());
}


long ExtControlsFrame::GetPhraseStart()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_start_phrase->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_start_phrase->text());
}


long ExtControlsFrame::GetPhraseEnd()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_end_phrase->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_end_phrase->text());
}


QString ExtControlsFrame::GetPhraseName()
{
	return ui.lineEdit_phrase_name->toPlainText();
}



void ExtControlsFrame::OnSelectedChanged(long startSample, long endSample)
{
	if ( GetFormatType() == Samples )
	{
		ui.lineEdit_start->setText(QString::number(startSample));
		ui.lineEdit_end->setText(QString::number(endSample));
		ui.lineEdit_len->setText(QString::number(endSample - startSample));
	}
	else
	{
		ui.lineEdit_start->setText(FromSampleToSecStr(startSample));
		ui.lineEdit_end->setText(FromSampleToSecStr(endSample));
		ui.lineEdit_len->setText(FromSampleToSecStr(endSample - startSample));
	}
}


void ExtControlsFrame::OnPlayPosChanged(long samples)
{
	if ( GetFormatType() == Samples )
	{
		ui.lineEdit_playPos->setText(QString::number(samples));
	}
	else
	{
		ui.lineEdit_playPos->setText(FromSampleToSecStr(samples));
	}
}


void ExtControlsFrame::OnMarkingChanged(const QString & markingName, 
						bool pahrasesIntersecton,
						const QColor & _color,
						const QString & fileName,
						const QString & maringStr)
{
	ui.lineEdit_markingName->blockSignals(true);
	ui.lineEdit_markingName->setText(markingName);
	ui.lineEdit_markingName->blockSignals(false);

	ui.lineEdit_markingFileName->blockSignals(true);
	ui.lineEdit_markingFileName->setText(fileName);
	ui.lineEdit_markingFileName->blockSignals(false);

	ui.checkBox_markingIntersection->blockSignals(true);
	ui.checkBox_markingIntersection->setChecked(pahrasesIntersecton);
	ui.checkBox_markingIntersection->blockSignals(false);

	color = _color;
	ui.lineEdit_markingValue->setText(maringStr);
}


void ExtControlsFrame::OnPhraseChanged(long startSample, long endSample, const QString & name)
{
	if ( GetFormatType() == Samples )
	{
		ui.lineEdit_start_phrase->setText(QString::number(startSample));
		ui.lineEdit_end_phrase->setText(QString::number(endSample));
		ui.lineEdit_len_phrase->setText(QString::number(endSample - startSample));
	}
	else
	{
		ui.lineEdit_start_phrase->setText(FromSampleToSecStr(startSample));
		ui.lineEdit_end_phrase->setText(FromSampleToSecStr(endSample));
		ui.lineEdit_len_phrase->setText(FromSampleToSecStr(endSample - startSample));	
	}
	ui.lineEdit_phrase_name->setText(name);
}



void ExtControlsFrame::OnPlayChanged(bool play)
{
	if ( ui.pushButton_playStop->isChecked() != play )
	{
		if ( play )
		{
			ui.pushButton_playStop->blockSignals(true);
			ui.pushButton_playStop->setChecked(true);
			ui.pushButton_playStop->blockSignals(false);
			ui.pushButton_pause->setEnabled(true);
		}
		else
		{
			ui.pushButton_playStop->blockSignals(true);
			ui.pushButton_playStop->setChecked(false);
			ui.pushButton_playStop->blockSignals(false);
			ui.pushButton_pause->setEnabled(false);	
		}
	}
}


void ExtControlsFrame::SetEnabledPlayback(bool enable)
{
	ui.groupBox_2->setEnabled(enable);
}


bool ExtControlsFrame::IsPlaybackEnabled()
{
	return ui.groupBox_2->isEnabled();
}

void ExtControlsFrame::SetEnabledPhrase(bool enabled)
{
	ui.groupBox_phrase->setEnabled(enabled);
}

bool ExtControlsFrame::IsPhraseEnabled()
{
	return ui.groupBox_phrase->isEnabled();
}

void ExtControlsFrame::SetPhraseNameFocus()
{
	ui.lineEdit_phrase_name->setFocus();
	ui.lineEdit_phrase_name->selectAll();
}

void ExtControlsFrame::SetEnabledMarking(bool enabled)
{
	ui.groupBox_marking->setEnabled(enabled);
}
bool ExtControlsFrame::IsMarkingEnabled()
{
	return ui.groupBox_marking->isEnabled();
}


//////////////////////////////////////////////////////////////////////////////////////////
void ExtControlsFrame::on_lineEdit_start_editingFinished()
{
	if ( GetFormatType() == Samples )
		emit StartSelectedChanged(ui.lineEdit_start->text().toLong());
	else
		emit StartSelectedChanged(FromSecStrToSample(ui.lineEdit_start->text()));
}

void ExtControlsFrame::on_lineEdit_end_editingFinished()
{
	if ( GetFormatType() == Samples )
		emit EndSelectedChanged(ui.lineEdit_end->text().toLong());
	else
		emit EndSelectedChanged(FromSecStrToSample(ui.lineEdit_end->text()));

}

void ExtControlsFrame::on_lineEdit_len_editingFinished()
{
	if ( GetFormatType() == Samples )
		emit EndSelectedChanged(ui.lineEdit_start->text().toLong() + 
								ui.lineEdit_len->text().toLong());
	else
	{
		long end = FromSecStrToSample(ui.lineEdit_start->text()) + 
			FromSecStrToSample(ui.lineEdit_len->text());
		emit EndSelectedChanged(end);
	}
}

void ExtControlsFrame::on_lineEdit_markingName_editingFinished()
{
	emit MarkingNameChanged(ui.lineEdit_markingName->text());
}


void ExtControlsFrame::on_pushButton_setColor_clicked()
{
	QColorDialog dialog;
	QColor curColor = dialog.getColor(color);
	
	if ( curColor != color )
	{
		color = curColor;
		emit MarkingColorChanged(color);
	}
}


void ExtControlsFrame::on_lineEdit_start_phrase_editingFinished()
{
	if ( GetFormatType() == Samples )
		emit StartPhraseChanged(ui.lineEdit_start_phrase->text().toLong());
	else
		emit StartPhraseChanged(FromSecStrToSample(ui.lineEdit_start_phrase->text()));
}

void ExtControlsFrame::on_lineEdit_end_phrase_editingFinished()
{
	if ( GetFormatType() == Samples )
		emit EndPhraseChanged(ui.lineEdit_end_phrase->text().toLong());
	else
		emit EndPhraseChanged(FromSecStrToSample(ui.lineEdit_end_phrase->text()));

}

void ExtControlsFrame::on_lineEdit_len_phrase_editingFinished()
{
	if ( GetFormatType() == Samples )
		emit EndPhraseChanged(ui.lineEdit_start_phrase->text().toLong() + 
								ui.lineEdit_len_phrase->text().toLong());
	else
	{
		long end = FromSecStrToSample(ui.lineEdit_start_phrase->text()) + 
			FromSecStrToSample(ui.lineEdit_len_phrase->text());
		emit EndPhraseChanged(end);
	}
}

void ExtControlsFrame::on_lineEdit_phrase_name_editingFinished()
{
	emit PhraseNameChanged(ui.lineEdit_phrase_name->toPlainText());
}


void ExtControlsFrame::on_checkBox_markingIntersection_toggled(bool checked)
{
	if ( checked && 
		QMessageBox::question(this, QString::fromLocal8Bit("Внимание!"), 
				QString::fromLocal8Bit("Фразы разметки могу быть удалены или значения фраз могут быть изменены. Продолжить?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No )
	{
		ui.checkBox_markingIntersection->blockSignals(true);
		ui.checkBox_markingIntersection->setChecked(false);
		ui.checkBox_markingIntersection->blockSignals(false);
	}
	emit MarkingIntersectionChanged(ui.checkBox_markingIntersection->isChecked());
}

void ExtControlsFrame::on_pushButton_playStop_toggled(bool checked)
{
	ui.pushButton_pause->setEnabled(checked);
	emit PlaybackChanged(checked);
}