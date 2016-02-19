#include "extlistwidget.h"

ExtListWidget::ExtListWidget(QWidget *parent)
	: QListWidget(parent),
	readonly(false)
{
}

ExtListWidget::~ExtListWidget()
{
}

void ExtListWidget::Init(const QVariantList & titles)
{
	QStringList sl;
	for( QVariantList::const_iterator iter = titles.begin();
		iter != titles.end(); iter++ )
		sl << iter->toString();
	Init(sl);
}


void ExtListWidget::Init(const QStringList & titles)
{
	QStringList state = GetSelected();
	clear();

	if ( readonly )
	{
		for ( QStringList::const_iterator iter = titles.begin();
			iter != titles.end(); iter++)
		{
			QListWidgetItem * item = new QListWidgetItem(*iter);
			item->setFlags(Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Unchecked);
			addItem(item);
		}
	}
	else
	{
		for ( QStringList::const_iterator iter = titles.begin();
			iter != titles.end(); iter++)
		{
			QListWidgetItem * item = new QListWidgetItem(*iter);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(Qt::Unchecked);
			addItem(item);
		}	
	}
	SetSelected(state);
}

void ExtListWidget::SetSelected(const QStringList & selected)
{
	for ( int i = 0; i < count(); i++ )
	{
		item(i)->setCheckState(Qt::Unchecked);

		for ( QStringList::const_iterator iter = selected.begin();
			iter != selected.end(); iter++ )
		{
			if( item(i)->text() == *iter )
			{
				item(i)->setCheckState(Qt::Checked);
				break;
			}
		}
	}
}

void ExtListWidget::SetSelected(const QVariantList & _selected)
{
	QStringList selected;
	foreach(const QVariant & s, _selected )
		selected << s.toString();
	SetSelected(selected);
}

QStringList ExtListWidget::GetSelected()const
{
	QStringList result;
	for ( int i = 0; i < count(); i++ )
		if ( item(i)->checkState() == Qt::Checked )
			result << item(i)->text();
	
	return result;
}
QList<QPair<QString, bool> > ExtListWidget::GetDataState() const
{
	QList<QPair<QString, bool> > result;

	for ( int i = 0; i < count(); i++ )
		result << QPair<QString, bool>(item(i)->text(),item(i)->checkState() == Qt::Checked);
	return result;
}


void ExtListWidget::SetReadonly(bool _readonly)
{
	if ( _readonly != readonly )
	{
		int flag = readonly ? (~Qt::ItemIsEnabled) : Qt::ItemIsEnabled;
		for ( int i = 0; i < count(); i++ )
			item(i)->setFlags(item(i)->flags() & flag);
	}
}