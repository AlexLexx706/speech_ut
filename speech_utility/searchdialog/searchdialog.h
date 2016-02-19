#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "ui_searchdialog.h"

class SearchDialog : public QDialog
{
	Q_OBJECT

public:
	enum SearchType
	{
		SearchInCurrentList,
		SearchInAll,
	};

	SearchDialog(QWidget *parent = 0);

	~SearchDialog();

	void SetSearchName(const QString & name);

	QString GetSearchName() const;

	SearchType GetSearchType() const;

	bool IsCaseSensitive() const;

	bool IsWholeWord() const;

signals:
	void FindNext();
private:
	Ui::SearchDialogClass ui;
};

#endif // SEARCHDIALOG_H
