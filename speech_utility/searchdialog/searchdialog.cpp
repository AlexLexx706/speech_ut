#include "searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton_findNext, SIGNAL(clicked()), this, SIGNAL(FindNext()));
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::SetSearchName(const QString & name)
{
	ui.comboBox_searchName->setEditText(name);
}


QString SearchDialog::GetSearchName() const
{
	return ui.comboBox_searchName->currentText();
}

SearchDialog::SearchType SearchDialog::GetSearchType() const
{
	if ( ui.comboBox_searchType->currentIndex() == 0 )
		return SearchInCurrentList;
	else
		return SearchInAll;
}

bool SearchDialog::IsCaseSensitive() const
{
	return ui.checkBox_caseSensitive->isChecked();
}

bool SearchDialog::IsWholeWord() const
{
	return ui.checkBox_wholeWord->isChecked();
}