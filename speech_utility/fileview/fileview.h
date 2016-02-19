#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QFrame>
#include <QFileInfo>
#include "../speech_utility_global.h"

class PhraseController;
class WaveViewerWidget;

class SPEECH_UTILITY_EXPORT FileView : public QFrame
{
	Q_OBJECT

public:
	FileView(QWidget *parent = 0);

	~FileView();

	PhraseController * GetPhraseController() const;

	WaveViewerWidget * GetWaveWidget() const;

	void SetFileName(const QString & _fileName);

	QString GetFileName()const {return fileName;}

	QString GetShortFileName()const {return QFileInfo(fileName).fileName();}

	void SetSignalDesc(const QString & desc){signalDesc.second = desc;}
	
	void SetSignalDescFilePath(const QString & file_path){signalDesc.first = file_path;}
	
	QPair<QString, QString> GetSignalDesc(){return signalDesc;}

	void setPhraseControllerUp(bool up);

signals:
	//! Изменения выделенных фреймов.
	void SelectedSamplesChanged(long startFrame, long endFrame);

	//! Позиция изменилась
	void PosChanged(long sample, double amplitude);

	//! бросили файл на окно.
	void DropWavFile(const QString & fileName);

	////////////////////////////////////////////////////////////////////

	//! Выбрали другую фразу или изменили границы.
	void CurrentPhraseChanged();

	//! Данные фразы изменились.
	void PhraseDataChanged();

	//! Выбрали другой лист.
	void CurrentListChanged();

	//! Данные листа изменились.
	void ListDataChanged();

	//! На фразе щёлкнули дважды.
	void PhraseDoubleClick();

	//! Бросили файл разметки.
	void DropMarkingFile(const QString & fileName);

	//! Сигнал изменился
	void DataChanged(long);

	void FieNameChanged(const QString & name);

private:
	void * __ui;
	QString fileName;

	QPair<QString, QString> signalDesc;
};

#endif // FILEVIEW_H
