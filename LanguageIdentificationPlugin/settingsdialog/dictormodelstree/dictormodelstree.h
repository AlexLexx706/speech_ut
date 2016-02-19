#ifndef DICTORMODELSTREE_H
#define DICTORMODELSTREE_H

#include <QTreeWidget>

class DictorModelsTree : public QTreeWidget
{
	Q_OBJECT

public:
	typedef QString ToolTipString;

	DictorModelsTree(QWidget *parent = 0);
	~DictorModelsTree();
	void Init(const QMap<QString, QStringList> & channels);
	void Init(const QVariantMap & channels);
	void SetModels(const QString & chName, const QStringList & models);
	void SetModels(const QString & chName, const QVariantList & models);
	QPair<QString, QStringList> GetCurModels() const;
	void UseMultiSelections(bool use){multiSelections = use;};
	void SetReadonly(bool readOnly);
	bool IsReadonly() const{return readonly;};
	void SetToolTips(const QMap<QString, QPair<ToolTipString, QMap<QString, ToolTipString> > > & toolTips);

private slots:
	void onItemChanged(QTreeWidgetItem * item, int column);
private:
	struct SpeakerModelInfo
	{
		int ModelID;
		QString ModelName;
		int SpeakerID;
		QString SpeakerName;
	};
	bool multiSelections;
	bool readonly;
	QMap<QString, SpeakerModelInfo> GetSpeakerModelInfo(const QStringList & models);
};

#endif // DICTORMODELSTREE_H
