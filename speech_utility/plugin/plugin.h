#ifndef _PLUGIN_ALEX_H_
#define _PLUGIN_ALEX_H_

#include "../BaseSUPlugin/plugincommon.h"
#include <QLibrary>

class FileView;
class QMenu;
class QSettings;
class InternalThread;
class QToolBar;

//! Реализует интерфейс для работы с плагином.
class PluginWrapper:public QObject
{
Q_OBJECT
public:
	//! Дополнительные данные.
	struct AdditionData
	{
		FileView * view;
		long startSample;
	};

	//! Конструктор.
	PluginWrapper(QObject * parent = NULL);
	
	//! Деструктор.
	~PluginWrapper();

	//! Получить плагин.
	SUPluginInterface * GetPlugin() const {return plugin;};

	//! Загрузить плагин из файла.
	static PluginWrapper * CreatePlugin(const QString & fileName, QSettings & settings, QMenu * menu, QToolBar * toolBar);

private:
	QMenu * menu;
	QLibrary lib;
	SUPluginInterface * plugin;

	//! Инициализация плагина.
	bool Init(const QString & fileName, QSettings & settings, QMenu * menu, QToolBar * toolBar);
};


#endif //_PLUGIN_ALEX_H_