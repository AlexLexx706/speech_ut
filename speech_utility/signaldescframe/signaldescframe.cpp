#include "signaldescframe.h"
#include <QDebug>
#include <QFile>
#include <string>
#include <fstream>

SignalDescFrame::SignalDescFrame(QWidget *parent)
	: QFrame(parent)
{
	ui.setupUi(this);
	connect(ui.radioButton_gender_male, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	connect(ui.radioButton_gender_female, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	connect(ui.radioButton_gender_child, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	connect(ui.radioButton_native_speaker, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));

	connect(ui.radioButton_age_5_7, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	connect(ui.radioButton_age_8_13, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	connect(ui.radioButton_age_13_more, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	connect(ui.radioButton_age_20_more, SIGNAL(toggled(bool)), this, SIGNAL(DataChanged()));
	
	connect(ui.checkBox_not_harmonic_noise, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
	connect(ui.checkBox_harmonic_noise, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
	connect(ui.checkBox_no_speech, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
	connect(ui.checkBox_bad_quality, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
	connect(ui.checkBox_unknown_lang, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_recorded_speech, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));

    connect(ui.checkBox_clipping, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_tv, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_children, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_people, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_music, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_echo, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_street, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    connect(ui.checkBox_metro, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
	connect(ui.checkBox_transport, SIGNAL(stateChanged(int)), this, SIGNAL(DataChanged()));
    ui.radioButton_gender_child->hide();

	SetDefaults();
}

SignalDescFrame::~SignalDescFrame()
{
}

void SignalDescFrame::SetGender(const std::string & gender)
{
	if ( gender == "Male" )
			ui.radioButton_gender_male->setChecked(true); 
	//else if ( gender == "Female" )
	else
		ui.radioButton_gender_female->setChecked(true); 
	//else
	//	ui.radioButton_gender_child->setChecked(true); 
}
std::string SignalDescFrame::GetGender() const
{
	if ( ui.radioButton_gender_male->isChecked() )
		return "Male";
	//else if (ui.radioButton_gender_female->isChecked() )
	else
		return "Female";
	//else 
	//	return "Child";
}

void SignalDescFrame::SetAge(const std::string & age)
{
	if ( age == "Small-kid" )
		ui.radioButton_age_5_7->setChecked(true);
	else if ( age == "Child" )
		ui.radioButton_age_8_13->setChecked(true);
	else if ( age == "Teen" )
		ui.radioButton_age_13_more->setChecked(true);
	else
		ui.radioButton_age_20_more->setChecked(true);
}

std::string SignalDescFrame::GetAge() const
{
	if ( ui.radioButton_age_5_7->isChecked() )
		return "Small-kid";
	else if ( ui.radioButton_age_8_13->isChecked() )
		return "Child";
	else if ( ui.radioButton_age_13_more->isChecked() )
		return "Teen";
	else
		return "Adult";
}


void SignalDescFrame::SetNative(bool native)
{
	if (native)
		ui.radioButton_native_speaker->setChecked(true);
	else
		ui.radioButton_no_native_speaker->setChecked(true);
}


bool SignalDescFrame::SaveToFile(const QString & file_path)
{
	QFile file(file_path);

	if ( file.open(QIODevice::WriteOnly) )
	{
		std::string str = json_spirit::write(GetDescJS(), json_spirit::pretty_print | json_spirit::raw_utf8);
		file.write(str.data(), str.size());
		file.close();
		return true;
	}
	else 
		qWarning() << "Cannot open file:" << file_path;
	return false;
}

bool SignalDescFrame::LoadFromFile(const QString & file_path)
{
	QFile file(file_path);

	if ( file.open(QIODevice::ReadOnly) )
	{
		QByteArray arr = file.readAll();
		std::string str(arr.data(), arr.size());
		file.close();
		json_spirit::mValue v;

		if ( json_spirit::read(str, v) )
			return SetDescJS(v);
		else
			qWarning() << "Cannot parse json file:" << file_path;
	}
	else 
		qWarning() << "Cannot open file:" << file_path;

	return false;
}

json_spirit::mValue SignalDescFrame::GetDescJS() const
{
	json_spirit::mObject obj;
	obj["gender"] = GetGender();
	obj["age_group"] = GetAge();
	obj["native"] = ui.radioButton_native_speaker->isChecked();
	obj["bad_quality_set"] = ui.checkBox_bad_quality->isChecked();
	obj["harmonic_noise_set"] = ui.checkBox_harmonic_noise->isChecked();
	obj["not_harmonic_noise_set"] = ui.checkBox_not_harmonic_noise->isChecked();
	obj["no_speech_set"] = ui.checkBox_no_speech->isChecked();
	obj["unknown_lang_set"] = ui.checkBox_unknown_lang->isChecked();
    obj["recorded_speech"] = ui.checkBox_recorded_speech->isChecked();
	
	obj["clipping"] = ui.checkBox_clipping->isChecked();
	obj["tv"] = ui.checkBox_tv->isChecked();
	obj["children"] = ui.checkBox_children->isChecked();
	obj["people"] = ui.checkBox_people->isChecked();
	obj["music"] = ui.checkBox_music->isChecked();
	obj["echo"] = ui.checkBox_echo->isChecked();
	obj["street"] = ui.checkBox_street->isChecked();
	obj["metro"] = ui.checkBox_metro->isChecked();
	obj["transport"] = ui.checkBox_transport->isChecked();


	return obj;
}

bool SignalDescFrame::SetDescJS(json_spirit::mValue v)
{
	try
	{
		json_spirit::mObject & obj = v.get_obj();
		json_spirit::mObject::iterator iter;

		bool bsb = blockSignals(true);
		SetGender(obj["gender"].get_str());
		SetAge(obj["age_group"].get_str());
		SetNative(obj["native"].get_bool());
		ui.checkBox_bad_quality->setChecked(obj["bad_quality_set"].get_bool());
		ui.checkBox_harmonic_noise->setChecked(obj["harmonic_noise_set"].get_bool());
		ui.checkBox_not_harmonic_noise->setChecked(obj["not_harmonic_noise_set"].get_bool());
		ui.checkBox_no_speech->setChecked(obj["no_speech_set"].get_bool());
		ui.checkBox_unknown_lang->setChecked(obj["unknown_lang_set"].get_bool());
        ui.checkBox_recorded_speech->setChecked(obj["recorded_speech"].get_bool());

		ui.checkBox_clipping->setChecked((iter = obj.find("clipping"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_tv->setChecked((iter = obj.find("tv"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_children->setChecked((iter = obj.find("children"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_people->setChecked((iter = obj.find("people"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_music->setChecked((iter = obj.find("music"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_echo->setChecked((iter = obj.find("echo"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_street->setChecked((iter = obj.find("street"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_metro->setChecked((iter = obj.find("metro"), iter != obj.end()) ? iter->second.get_bool() : false);
		ui.checkBox_transport->setChecked((iter = obj.find("transport"), iter != obj.end()) ? iter->second.get_bool() : false);

		blockSignals(bsb);
		return true;
	}
	catch(std::exception & e)
	{
		qWarning() << "Error:" << e.what();
	}
	return false;
}

QString SignalDescFrame::GetDesc() const
{
	return QString::fromStdString(json_spirit::write(GetDescJS(), json_spirit::pretty_print | json_spirit::raw_utf8));
}

void SignalDescFrame::SetDesc(const QString & desc)
{
	json_spirit::mValue v;

	if ( json_spirit::read(desc.toStdString(), v) )
		SetDescJS(v);
	else
		SetDefaults();
}

void SignalDescFrame::SetDefaults()
{
	bool bsb = blockSignals(true);
	//SetGender("Child");
	SetGender("Male");
	//SetAge("Child");
	SetAge("Adult");
	SetNative(true);
	ui.checkBox_bad_quality->setChecked(false);
	ui.checkBox_harmonic_noise->setChecked(false);
	ui.checkBox_not_harmonic_noise->setChecked(true);
	ui.checkBox_no_speech->setChecked(false);
	ui.checkBox_unknown_lang->setChecked(false);
    ui.checkBox_recorded_speech->setChecked(false);

	ui.checkBox_clipping->setChecked(false);
	ui.checkBox_tv->setChecked(false);
	ui.checkBox_children->setChecked(false);
	ui.checkBox_people->setChecked(false);
	ui.checkBox_music->setChecked(false);
	ui.checkBox_echo->setChecked(false);
	ui.checkBox_street->setChecked(false);
	ui.checkBox_metro->setChecked(false);
	ui.checkBox_transport->setChecked(false);
	blockSignals(bsb);
}