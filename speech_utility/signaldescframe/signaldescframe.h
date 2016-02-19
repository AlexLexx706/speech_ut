#ifndef SIGNALDESCFRAME_H
#define SIGNALDESCFRAME_H

#include <QFrame>
#include "ui_signaldescframe.h"
#include "../json_spirit/json_spirit.h"

class SignalDescFrame : public QFrame
{
	Q_OBJECT

public:
	SignalDescFrame(QWidget *parent = 0);

	~SignalDescFrame();

	bool LoadFromFile(const QString & file_path);
	
	bool SaveToFile(const QString & file_path);

	QString GetDesc() const;

	void SetDesc(const QString & desc);

signals:
	void DataChanged();

private:
	Ui::SignalDescFrame ui;

	void SetGender(const std::string & gender);

	std::string GetGender() const;

	void SetAge(const std::string & age);

	std::string GetAge() const;

	void SetNative(bool native);

	json_spirit::mValue GetDescJS() const;

	bool SetDescJS(json_spirit::mValue v);

	void SetDefaults();
};

#endif // SIGNALDESCFRAME_H
