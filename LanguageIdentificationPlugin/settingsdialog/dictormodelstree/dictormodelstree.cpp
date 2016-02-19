#include "dictormodelstree.h"
#include <QTreeWidgetItem>

DictorModelsTree::DictorModelsTree(QWidget *parent)
	: QTreeWidget(parent),
	multiSelections(false),
	readonly(false)
{
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
		this, SLOT(onItemChanged(QTreeWidgetItem *, int)));

}

DictorModelsTree::~DictorModelsTree()
{

}

void DictorModelsTree::Init(const QVariantMap & _channels)
{
	QMap<QString, QStringList> channels;
	
	for (QVariantMap::const_iterator iter = _channels.begin(); 
		iter != _channels.end(); iter++ )
	{
		QVariantList _list = iter->toList();
		QStringList & list = channels[iter.key()];
		
		for ( QVariantList::iterator mIter = _list.begin();
			mIter != _list.end(); mIter++ )
			list << mIter->toString();
	}
	Init(channels);
}

void DictorModelsTree::Init(const QMap<QString, QStringList> & channels)
{
	QPair<QString, QStringList> curParams = GetCurModels();

	clear();
	setHeaderHidden(true);

	blockSignals(true);
	for ( QMap<QString, QStringList>::const_iterator iter = channels.constBegin();
		iter != channels.constEnd(); iter++ )
	{
		QTreeWidgetItem * chItem = new QTreeWidgetItem(QStringList() <<  iter.key());
		chItem->setFlags(Qt::ItemIsSelectable|
						(readonly ? Qt::NoItemFlags :Qt::ItemIsEnabled));
		addTopLevelItem(chItem);

		foreach(const QString & model, *iter)
		{
			QTreeWidgetItem * mItem = new QTreeWidgetItem(QStringList() << model);
			mItem->setData(0, Qt::UserRole, model);
			mItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |
				(readonly ? Qt::NoItemFlags :Qt::ItemIsEnabled));
			chItem->addChild(mItem);
		}		
	}
	blockSignals(false);
	SetModels(curParams.first, curParams.second);
	expandAll();
}

void DictorModelsTree::SetModels(const QString & chName, const QStringList & models)
{
	QTreeWidgetItem * chItem;
	QTreeWidgetItem * mItem;

	for (int i = 0; i < topLevelItemCount(); i++ )
	{
		chItem = topLevelItem(i);
		if ( chItem->text(0) == chName )
		{
			chItem->setCheckState(0, Qt::Checked);

			for( int i = 0; i < chItem->childCount(); i++)
			{
				QTreeWidgetItem * mItem = chItem->child(i);
				Qt::CheckState checked = Qt::Unchecked;

				foreach(const QString & mName, models)
				{
					if ( mName == mItem->data(0, Qt::UserRole).toString() )
					{
						checked = Qt::Checked;
						break;
					}
				}
				mItem->setCheckState(0, checked);
			}
			return;
		}
	}
	
	for (int i = 0; i < topLevelItemCount(); i++ )
		topLevelItem(i)->setCheckState(0, Qt::Unchecked);
}

void DictorModelsTree::SetModels(const QString & chName, const QVariantList & _models)
{
	QStringList models;
	foreach(const QVariant & v, _models)
		models << v.toString();
	
	SetModels(chName, models);
}

QPair<QString, QStringList> DictorModelsTree::GetCurModels() const
{
	QString curChannel;
	QStringList curModels;

	QTreeWidgetItem * chItem;
	QTreeWidgetItem * mItem;

	for (int i = 0; i < topLevelItemCount(); i++ )
	{
		chItem = topLevelItem(i);
		if ( chItem->checkState(0) == Qt::Checked )
		{
			curChannel = chItem->text(0);
			for( int i = 0; i < chItem->childCount(); i++)
				if( chItem->child(i)->checkState(0) == Qt::Checked )
					curModels << chItem->child(i)->data(0, Qt::UserRole).toString();
		}
	}
	return QPair<QString, QStringList>(curChannel, curModels);
}

void DictorModelsTree::SetReadonly(bool _readonly)
{
	if ( _readonly != readonly )
	{
		readonly = _readonly;

		QTreeWidgetItem * chItem;
		QTreeWidgetItem * mItem;
		int flag = readonly ? (~Qt::ItemIsEnabled) : Qt::ItemIsEnabled;

		for (int i = 0; i < topLevelItemCount(); i++ )
		{
			chItem = topLevelItem(i);
			chItem->setFlags(chItem->flags() & flag);
			
			for( int i = 0; i < chItem->childCount(); i++)
			{
				QTreeWidgetItem * mItem = chItem->child(i);
				mItem->setFlags(mItem->flags() & flag);
			}
		}	
	}
}


void DictorModelsTree::SetToolTips(const QMap<QString, QPair<ToolTipString, QMap<QString, ToolTipString> > > & toolTips)
{
	QTreeWidgetItem * chItem;
	QTreeWidgetItem * mItem;
	QMap<QString, QPair<ToolTipString, QMap<QString, ToolTipString> > >::const_iterator chIter;
	QMap<QString, ToolTipString>::const_iterator mIter;

	for (int i = 0; i < topLevelItemCount(); i++ )
	{
		chItem = topLevelItem(i);
		chIter = toolTips.find(chItem->text(0));
		
		if ( chIter != toolTips.end() )
		{
			chItem->setToolTip(0, chIter->first);

			for( int i = 0; i < chItem->childCount(); i++)
			{
				mItem = chItem->child(i);
				mIter = chIter->second.find(mItem->text(0));
				
				if ( mIter != chIter->second.end() )
					mItem->setToolTip(0, *mIter);
			}
		}
	}
}



void DictorModelsTree::onItemChanged(QTreeWidgetItem * item, int)
{
	if ( item )
	{
		blockSignals(true);
		if ( QTreeWidgetItem * parent = item->parent())
		{
			//Проверяем
			if ( item->checkState(0) == Qt::Checked )
			{
				if ( multiSelections )
				{
					for (int i = 0; i < topLevelItemCount(); i++ )
					{
						QTreeWidgetItem * chItem = topLevelItem(i);
						if ( chItem != parent )
						{
							chItem->setCheckState(0, Qt::Unchecked);
							for( int i = 0; i < chItem->childCount(); i++)
								chItem->child(i)->setCheckState(0, Qt::Unchecked);
						}
						else
							chItem->setCheckState(0, Qt::Checked);
					}
				}
				else
				{
					for (int i = 0; i < topLevelItemCount(); i++ )
					{
						QTreeWidgetItem * chItem = topLevelItem(i);
						if ( chItem != parent )
						{
							chItem->setCheckState(0, Qt::Unchecked);
							for( int i = 0; i < chItem->childCount(); i++)
								chItem->child(i)->setCheckState(0, Qt::Unchecked);
						}
						//Отменим все остальные.
						else
						{
							chItem->setCheckState(0, Qt::Checked);

							for( int i = 0; i < chItem->childCount(); i++)
								if ( chItem->child(i) != item )
									chItem->child(i)->setCheckState(0, Qt::Unchecked);
						}
					}				
				}
			}
		}
		else
		{
			//Проверяем
			if ( item->checkState(0) == Qt::Checked )
			{
				for (int i = 0; i < topLevelItemCount(); i++ )
				{
					QTreeWidgetItem * chItem = topLevelItem(i);
					if ( chItem != item )
					{
						chItem->setCheckState(0, Qt::Unchecked);
						for( int i = 0; i < chItem->childCount(); i++)
							chItem->child(i)->setCheckState(0, Qt::Unchecked);
					}
					else
						chItem->setCheckState(0, Qt::Checked);
				}
			}
			else
				for( int i = 0; i < item->childCount(); i++)
					item->child(i)->setCheckState(0, Qt::Unchecked);			
		}
		blockSignals(false);
	}
}