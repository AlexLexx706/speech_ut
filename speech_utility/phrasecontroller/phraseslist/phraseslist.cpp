#include "phraseslist.h"
#include <QLabel>
#include <QCheckBox>
#include "phraseslistdata/phraseslistdata.h"
#include <QHBoxLayout>
#include "../phrasecontroller.h"
#include "ui_phraseslist.h"

PhrasesList::PhrasesList(PhraseController & _controller, QList<Phrase *> & data):
	QFrame((QWidget *)&_controller),
	controller(_controller)
{
	ui = new Ui::Frame;
	((Ui::Frame * )ui)->setupUi(this);
	connect(((Ui::Frame * )ui)->checkBoxEnable, SIGNAL(stateChanged(int)), this, SLOT(OnStateChanged(int)));
	connect(this, SIGNAL(ListDataChanged()), &controller, SIGNAL(ListDataChanged()));
}

PhrasesList::~PhrasesList()
{
}

PhrasesListData & PhrasesList::GetData() const
{
	return *((Ui::Frame * )ui)->list;
}

void PhrasesList::SetName(const QString & _name)
{
	if ( _name != name )
	{
		name = _name;
		((Ui::Frame * )ui)->list->setToolTip(QString::fromLocal8Bit("Разметка:%1").arg(name));
		emit ListDataChanged();
	}
}

void PhrasesList::SetFileName(const QString & _fileName )
{
	if ( _fileName != fileName )
	{
		fileName = _fileName;
		emit ListDataChanged();
	}
}



void PhrasesList::SetEnable(bool enable)
{
	((Ui::Frame * )ui)->checkBoxEnable->setChecked(enable);
	((Ui::Frame * )ui)->list->setEnabled(enable);
	((Ui::Frame * )ui)->list->SetButtonsVisible(enable);
	emit ListDataChanged();
}

bool PhrasesList::IsEnable()
{
	return ((Ui::Frame * )ui)->checkBoxEnable->isChecked();
}


void PhrasesList::OnStateChanged(int state)
{
	SetEnable(state == Qt::Checked);
}
