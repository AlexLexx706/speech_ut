#ifndef FILEVIEWCONTROLLER_H
#define FILEVIEWCONTROLLER_H

#include <QTabWidget>
#include "../speech_utility_global.h"

class FileView;
class PhraseController;
class WaveViewerWidget;
class DataFrame;
class PhrasesList;
class PhrasesListData;
class Phrase;
class QMenu;

class SPEECH_UTILITY_EXPORT FileViewController : public QTabWidget
{
	Q_OBJECT

public:
	FileViewController(QWidget *parent = 0);
	
	~FileViewController();
	
	FileView * GetCurFileView() const;

	PhraseController * GetCurPhraseController() const;

	WaveViewerWidget * GetCurWaveWidget() const;

	DataFrame * GetCurDataFrame() const;

	PhrasesList * GetCurPhrasesList() const;

	PhrasesListData * GetCurPhrasesListData() const;

	Phrase * GetCurPhrase() const;

	void SetCurFileView(FileView * file);

	FileView * AddFileView(const QString & fileName);

	void DelFileView(FileView * file);

	void SetPhrasesListMenu(QMenu * menu) { phrasesListMenu = menu;}

	void SetMenuView(QMenu * menu) { menuView = menu;}

	void SetMenuEdit(QMenu * menu) { menuEdit = menu;}

	void ClearAll();

	bool IsExist(FileView * view) const;

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

	//! Сигнал изменился.
	void DataChanged(long);

protected:
	void dragEnterEvent(QDragEnterEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void dragLeaveEvent(QDragLeaveEvent *event);

	void dropEvent(QDropEvent *event);

private:
	void * _ui;
	QMenu * phrasesListMenu;
	QMenu * menuView;
	QMenu * menuEdit;

private slots:
	void onFieNameChanged(const QString & name);


};

#endif // FILEVIEWCONTROLLER_H
