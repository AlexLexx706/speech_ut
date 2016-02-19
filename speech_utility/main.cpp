#include <qapplication.h>
#include "mainwindow.h" 
#include <string>
#include <iostream>
#include <QDir>
#include <fcntl.h>     /* for _O_TEXT and _O_BINARY */
#include <io.h>
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <fstream>
#include <QDebug>
#include <QTextStream>
#include <cstdio>
#include <QTextCodec>


void myMessageOutput(QtMsgType type, const char *msg){std::cout << msg;}
void myMessageOutputNoOut(QtMsgType type, const char *msg){}

bool ConnectToConsole()
{
	if ( GetStdHandle(STD_OUTPUT_HANDLE) == NULL )
		AllocConsole();
	else
		AttachConsole(ATTACH_PARENT_PROCESS);
	freopen("CONOUT$", "w", stdout);
	return true;
}



int main (int argc, char **argv)
{
	qInstallMsgHandler(myMessageOutput);

	QApplication a(argc, argv);

	QStringList args = QCoreApplication::arguments();

	bool onlyConsole = false;
	QVariantMap vMap;
	vMap["usignMode"] = MainWindow::Normal;

	args.pop_front();
	if ( !args.isEmpty() )
	{
		QSet<QString> enabled_keys; enabled_keys <<"/?" << "/C" << "/I" << "/D" << "/O" << "/MSL" << "/DC" << "/XML" << "/SPV";
		QMap<QString, QStringList> keys;
		QStringList * values = NULL;

		//Сборка ключей.
		foreach(const QString & str, args)
		{
			if ( enabled_keys.contains(str) )
				values = &keys[str];
			else
			{
				if (!values)
					values = &keys["/I"];
				values->append(str);
			}
		}

		//Установка опций.
		for (QMap<QString, QStringList>::iterator keyIter = keys.begin();
			keyIter != keys.end(); keyIter++ )
		{
			//! Входные файлы.
			if ( keyIter.key() == "/I" && keyIter->size() )
			{
				QVariantList vList;

				foreach(QString str, *keyIter)
					vList << str;
				vMap["inputFiles"] = vList;
			}
			//! Выходной файл.
			else if ( keyIter.key() == "/O" && keyIter->size() )
			{
				vMap["outFile"] = keyIter->first();
			}
			else if ( keyIter.key() == "/MSL" && keyIter->size())
			{
				vMap["minSpeechDur"] = keyIter->first().toFloat();
			}
			else if ( keyIter.key() == "/DC" && keyIter->size())
			{
				vMap["useDictorCount"] = bool(keyIter->first().toInt());
			}
			else if ( keyIter.key() == "/XML" && keyIter->size())
			{
				vMap["makeXml"] = bool(keyIter->first().toInt());
			}
			else if ( keyIter.key() == "/D" && keyIter->size())
			{
				QStringList files = QDir(keyIter->first()).entryList(QStringList() << "*.wav" << "*.WAV", QDir::Files);

				if( !files.size() )
					std::cout << "No have input files!!!";

				QVariantList vList;
				
				for ( QStringList::iterator iter = files.begin(); 
						iter != files.end(); iter++ )
					vList << QString("%1/%2").arg(keyIter->first()).arg(*iter);

				if ( vList.size() )
				{
					if( vMap["inputFiles"].isValid() )
						vList += vMap["inputFiles"].toList();
					vMap["inputFiles"] = vList;
				}
			}
			//! Режим консоли.
			else if ( keyIter.key() == "/C" )
			{
				onlyConsole = true;
				ConnectToConsole();
				qInstallMsgHandler(myMessageOutputNoOut);

			}
			else if ( keyIter.key() == "/SC" )
			{
				ConnectToConsole();
			}
			else if ( keyIter.key() == "/?" )
			{
				ConnectToConsole();
				setlocale(LC_ALL, "Russian");
				QTextStream out(stdout);
				out.setCodec("Cp866");

				out << QObject::trUtf8("Обрабатывает файлы плагином VAD.") << endl;
				out << endl;
				out << QObject::trUtf8("[список файлов для просмотра] [/C] [/I список файлов] [/D путь к папке] [/O выходной файл] [/MSL минимальная дилительность речи] [/DC проверять количество дикторов]") << endl;
				out << endl;
				out << QObject::trUtf8(" /C \tПрограмма работает в консольном режиме") << endl;
				out << QObject::trUtf8(" /I \tСписок файлов разделённых пробелами, например c:\\1.wav c:\\temp\\2.wav") << endl;
				out << QObject::trUtf8(" /D \tПапка с файлами на обработку, из папки считываются все файлы расширеня wav, например c:\\files") << endl;
				out << QObject::trUtf8(" /O \tФайл результат, например c:\\out.wav, программа не создаёт директорию для данного файла!!!!") << endl;
				out << QObject::trUtf8(" /MSL \tминимальная дилительность речи сек., по умолчанию 20. сек.") << endl;
				out << QObject::trUtf8(" /DC \tПроверять количество дикторов: 0 или 1, по умолчанию отключено 0.") << endl;
				out << QObject::trUtf8(" /XML \tСоздаёт xml файл (0 или 1, по умолчанию отключено 0) содержащий разметку фрагментов речи, при этом обрабатываемый файл не модиффицируется.") << endl;
				
				out << endl;
				out << QObject::trUtf8("С параметрами запуска, программа выполняет следующие действия:") << endl;
				out << QObject::trUtf8(" 1) \tЗагружает входные файлы. /I /D") << endl;
				out << QObject::trUtf8(" 2) \tСклеивае файлы в один файл.") << endl;
				out << QObject::trUtf8(" 3) \tОбрабытывает файл плагином VAD, получает разметку") << endl;
				out << QObject::trUtf8(" 4) \tУдаляет из файла сигналы шума.") << endl;
				out << QObject::trUtf8(" 5) \tСохраняет обработанный файл по заданному пути /O.") << endl;
				out << endl << endl;

				out << QObject::trUtf8("Коды возврата:") << endl;
				out << QObject::trUtf8("0 - всё ок") << endl;
				out << QObject::trUtf8("1 - неизвестная ошибка") << endl;
				out << QObject::trUtf8("2 - мало речи") << endl;
				out << QObject::trUtf8("3 - количество речи меньше чем заданое") << endl;
				out << QObject::trUtf8("4 - обнаружены различные дикторы") << endl;

				return 0;
			}
			//перезапуск пакетной обработки вада.
			else if ( keyIter.key() == "/SPV")
			{
				vMap["startPacketVad"] = true;
			}
		}

		if ( vMap["outFile"].isValid() && vMap["inputFiles"].isValid() )
		{
			vMap["usignMode"] = onlyConsole ? MainWindow::ConsoleCutVad : MainWindow::CutVad;
		}
	}
	MainWindow w(vMap);

	if ( !onlyConsole )
	{
		w.resize(540,400);
		w.show();
	}
	return a.exec();
}
