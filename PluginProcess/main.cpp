#include <QtCore/QCoreApplication>
#include <QPluginLoader>
#include <QStringList>
#include <iostream>
#include "basepluginserver/basepluginserver.h"
#include "../BaseSUPlugin/plugincommon.h"
#include <Windows.h>
#include <QMetatype.h>


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	//! Отключим диалог сообщения об ошибки.
	::SetErrorMode(SEM_NOGPFAULTERRORBOX);

	std::cout << "Start process" << std::endl;

	if ( QCoreApplication::arguments().size() >= 3 )
	{
		QString pluginLib = QCoreApplication::arguments()[1];
		int port = QCoreApplication::arguments()[2].toInt();
		QHostAddress host(QCoreApplication::arguments().size() > 3 ? QHostAddress(QCoreApplication::arguments()[3]) : QHostAddress::Any);

		if ( !pluginLib.isEmpty() && port )
		{
			qRegisterMetaType<SUPPhraseData>("SUPPhraseData");
			qRegisterMetaType<QList<SUPPhraseData>>("QList<SUPPhraseData>");
			qRegisterMetaType<SUPFileResult>("SUPFileResult");
			qRegisterMetaType<SUPInputData>("SUPInputData");
			qRegisterMetaType<SUPOutData>("SUPOutData");

			QPluginLoader pluginLoader(pluginLib);

			if ( SUPluginInterface * plugin = qobject_cast<SUPluginInterface *>(pluginLoader.instance()) )
			{
				BasePluginServer server(*plugin);
				if ( server.listen(host, port) && server.IsInit() )
				{
					std::cout << "server on port: " << port << " started with: " << pluginLib.toLocal8Bit().data() << std::endl;
					int res = a.exec();
					std::cout << "Server stoped" << std::endl;
					return res;
				}
				else 
					std::cout << "cannot start server on port: " << port << std::endl;
			}
			else 
				std::cout << "plugin: " << pluginLib.toStdString() << " not loaded" << std::endl;
		}
		else 
			std::cout << "wrong argument!!!" << std::endl;
	}
	std::cout << "no argument!!!" << std::endl;
	return 0;
}
