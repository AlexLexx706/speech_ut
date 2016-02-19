#pragma once
#include <QString>
#include <QStringList>

class Process
{
public:
	Process(void);
	~Process(void);
	bool start(const QString & program, const QStringList & arguments);
	bool kill();
	QString error() const;
private:
	void * pd;
};
