#ifndef _PLUGIN_ALEX_H_
#define _PLUGIN_ALEX_H_

#include "../BaseSUPlugin/plugincommon.h"
#include <QLibrary>

class FileView;
class QMenu;
class QSettings;
class InternalThread;
class QToolBar;

//! ��������� ��������� ��� ������ � ��������.
class PluginWrapper:public QObject
{
Q_OBJECT
public:
	//! �������������� ������.
	struct AdditionData
	{
		FileView * view;
		long startSample;
	};

	//! �����������.
	PluginWrapper(QObject * parent = NULL);
	
	//! ����������.
	~PluginWrapper();

	//! �������� ������.
	SUPluginInterface * GetPlugin() const {return plugin;};

	//! ��������� ������ �� �����.
	static PluginWrapper * CreatePlugin(const QString & fileName, QSettings & settings, QMenu * menu, QToolBar * toolBar);

private:
	QMenu * menu;
	QLibrary lib;
	SUPluginInterface * plugin;

	//! ������������� �������.
	bool Init(const QString & fileName, QSettings & settings, QMenu * menu, QToolBar * toolBar);
};


#endif //_PLUGIN_ALEX_H_