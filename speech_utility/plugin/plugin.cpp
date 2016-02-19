#include "plugin.h"
#include <QMenu>
#include <QThread>
#include <QDialog>
#include <QMessageBox>
#include <QToolBar>


///////////////////////////////////////////////////////////////////////////////////////
PluginWrapper::PluginWrapper(QObject * parent):QObject(parent),
	menu(NULL),
	plugin(NULL)
{}

PluginWrapper::~PluginWrapper()
{
	delete menu;
}


bool PluginWrapper::Init(const QString & fileName, QSettings & settings, QMenu * inMenu, QToolBar * toolBar)
{
	if ( inMenu && toolBar )
	{
		lib.setFileName(fileName);
		pGetSUPliginFun fun;

		if (lib.load() && 
			(fun = (pGetSUPliginFun)lib.resolve("GetSUPligin")) &&
			(plugin = fun()) )
			return plugin->Init(settings, inMenu, toolBar);
	}
	return false;
}




PluginWrapper * PluginWrapper::CreatePlugin(const QString & fileName, QSettings & settings, QMenu * inMenu, QToolBar * toolBar)
{
	PluginWrapper * plugin = new PluginWrapper();

	if ( plugin->Init(fileName, settings, inMenu, toolBar) )
		return plugin;
	delete plugin;
	return NULL;
}
