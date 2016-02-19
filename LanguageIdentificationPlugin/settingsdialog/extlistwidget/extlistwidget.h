#ifndef EXTLISTWIDGET_H
#define EXTLISTWIDGET_H

#include <QListWidget>
#include <QStringList>
#include <QVariantList>

class ExtListWidget : public QListWidget
{
	Q_OBJECT

public:
	ExtListWidget(QWidget *parent = 0);
	~ExtListWidget();
	void Init(const QStringList & titles);
	void Init(const QVariantList & titles);
	void SetSelected(const QStringList & selected);
	void SetSelected(const QVariantList & selected);
	QStringList GetSelected()const;
	QList<QPair<QString, bool> > GetDataState() const;
	void SetReadonly(bool readonly);
	bool IsReadOnly() const{return readonly;}

private:
	bool readonly;
};

#endif // EXTLISTWIDGET_H
