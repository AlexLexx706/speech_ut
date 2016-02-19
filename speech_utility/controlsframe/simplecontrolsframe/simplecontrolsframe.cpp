#include "simplecontrolsframe.h"
#include "ui_simplecontrolsframe.h"
#include "../../../json_spirit/json_spirit.h"
#include <QDebug>


#define ui (*((Ui::SimpleControlsFrame *)_ui))

SimpleControlsFrame::SimpleControlsFrame(QWidget *parent):
	ControlsFrame(parent),
		_ui( new Ui::SimpleControlsFrame)
{
	ui.setupUi(this);
	connect(ui.pushButton_pause, SIGNAL(clicked()), this, SIGNAL(PauseClicked()));
}

SimpleControlsFrame::~SimpleControlsFrame()
{
	delete (Ui::SimpleControlsFrame *)_ui;
}

long SimpleControlsFrame::GetSelectedStart()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_start->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_start->text());
}

long SimpleControlsFrame::GetSelectedEnd()
{
	if ( GetFormatType() == Samples )
		return ui.lineEdit_end->text().toLong();
	else
		return FromSecStrToSample(ui.lineEdit_end->text());	
}

void SimpleControlsFrame::OnMarkingChanged(const QString & markingName, 
											bool pahrasesIntersecton,
											const QColor & color,
											const QString & fileName,
											const QString & maringStr)
{
	if ( maringStr != ui.lineEdit_markingName->text() )
		ui.lineEdit_markingName->setText(maringStr);
}

void SimpleControlsFrame::OnSelectedChanged(long start, long end)
{
	if ( start != GetSelectedStart() || 
		end != GetSelectedEnd() )
	{
		if ( GetFormatType() == Samples )
		{
			ui.lineEdit_start->setText(QString::number(start));
			ui.lineEdit_end->setText(QString::number(end));
			ui.lineEdit_length->setText(QString::number(end - start));
		}
		else
		{
			ui.lineEdit_start->setText(FromSampleToSecStr(start));
			ui.lineEdit_end->setText(FromSampleToSecStr(end));
			ui.lineEdit_length->setText(FromSampleToSecStr(end - start));
		}
	}
}

void SimpleControlsFrame::OnPlayChanged(bool play)
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

void SimpleControlsFrame::SetEnabledPlayback(bool enable)
{
	ui.frame_playback->setEnabled(enable);
}

bool SimpleControlsFrame::IsPlaybackEnabled()
{
	return ui.frame_playback->isEnabled();
}

void SimpleControlsFrame::SetConfig(const QString & jsonString)
{
	json_spirit::mValue v;
	if ( json_spirit::read(jsonString.toStdString(), v) )
	{
		json_spirit::mObject & obj = v.get_obj();
		json_spirit::mObject::const_iterator iter;

		if ( (iter = obj.find("show_marking")) != obj.end() )
		{
			ui.label_15->setVisible(iter->second.get_bool());
			ui.lineEdit_markingName->setVisible(iter->second.get_bool());
		}
	}
	else
		qWarning() << "Cannot read settings: " << jsonString;
}


void SimpleControlsFrame::on_pushButton_playStop_toggled(bool checked)
{
	ui.pushButton_pause->setEnabled(checked);
	emit PlaybackChanged(checked);
}