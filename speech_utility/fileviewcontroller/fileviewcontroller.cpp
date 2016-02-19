#include "fileviewcontroller.h"
#include "../fileview/fileview.h"
#include "../phrasecontroller/phraseslist/phraseslistdata/phraseslistdata.h"
#include <QDragEnterEvent>
#include <QUrl>
#include <QFileInfo>
#include "ui_fileviewcontroller.h"
#include "../waveviewerwidget/waveviewerwidget.h"
#include "../phrasecontroller/phrasecontroller.h"


#define ui (*((Ui::FileViewControllerClass *)_ui))

FileViewController::FileViewController(QWidget *parent)
	: QTabWidget(parent),
	phrasesListMenu(NULL),
	menuView(NULL),
	menuEdit(NULL)
{
	_ui = new Ui::FileViewControllerClass;
	ui.setupUi(this);
	tabBar()->setVisible(false);

	//! Удалим левые табы.
}

FileViewController::~FileViewController()
{
	delete (Ui::FileViewControllerClass *)_ui;
}

FileView * FileViewController::GetCurFileView() const
{
	return (FileView *)currentWidget();
}

PhraseController * FileViewController::GetCurPhraseController() const
{
	if ( FileView * fileView = (FileView *)currentWidget() )
		return fileView->GetPhraseController();
	return NULL;
}

WaveViewerWidget * FileViewController::GetCurWaveWidget() const
{
	if ( FileView * fileView = (FileView *)currentWidget() )
		return fileView->GetWaveWidget();
	return NULL;
}

DataFrame * FileViewController::GetCurDataFrame() const
{
	if ( FileView * fileView = (FileView *)currentWidget() )
		return fileView->GetWaveWidget()->GetWaveData();
	return NULL;
}

PhrasesList * FileViewController::GetCurPhrasesList() const
{
	if ( PhraseController * c = GetCurPhraseController() )
		return c->GetCurList();
	return NULL;
}

PhrasesListData * FileViewController::GetCurPhrasesListData() const
{
	if ( PhraseController * c = GetCurPhraseController() )
		return c->GetCurPhraseListData();
	return NULL;
}

Phrase * FileViewController::GetCurPhrase() const
{
	if ( PhrasesListData * ld = GetCurPhrasesListData() )
		return ld->GetCurPhrase();
	return NULL;
}


void FileViewController::SetCurFileView(FileView * file)
{
	setCurrentIndex(indexOf(file));
}

void FileViewController::onFieNameChanged(const QString & name)
{
	FileView * fw =  dynamic_cast<FileView *>(sender());
	
	if (fw)
	{
		int index = indexOf(fw);
		emit currentChanged(index);
		setTabText(index, fw->GetShortFileName());
	}
}

FileView * FileViewController::AddFileView(const QString & fileName)
{
	FileView * view = new FileView(this);

	connect(view, SIGNAL(SelectedSamplesChanged(long,long)), this, SIGNAL(SelectedSamplesChanged(long, long)) );
	connect(view, SIGNAL(PosChanged(long, double)), this, SIGNAL(PosChanged(long, double)) );
	connect(view, SIGNAL(DropWavFile(const QString &)), this, SIGNAL(DropWavFile(const QString &)) );

	connect(view, SIGNAL(CurrentPhraseChanged()), this, SIGNAL(CurrentPhraseChanged()) );
	connect(view, SIGNAL(PhraseDataChanged()), this, SIGNAL(PhraseDataChanged()) );
	connect(view, SIGNAL(CurrentListChanged()), this, SIGNAL(CurrentListChanged()) );
	connect(view, SIGNAL(ListDataChanged()), this, SIGNAL(ListDataChanged()) );
	connect(view, SIGNAL(PhraseDoubleClick()), this, SIGNAL(PhraseDoubleClick()) );
	connect(view, SIGNAL(DropMarkingFile(const QString & )), this, SIGNAL(DropMarkingFile(const QString & )) );
	connect(view, SIGNAL(DataChanged(long)), this, SIGNAL(DataChanged(long)));
	connect(view, SIGNAL(FieNameChanged(const QString &)), this, SLOT(onFieNameChanged(const QString &)));

	view->GetPhraseController()->SetPhrasesListMenu(phrasesListMenu);
	view->GetWaveWidget()->SetMenuEdit(menuEdit);
	view->GetWaveWidget()->SetMenuView(menuView);
	
	QFileInfo fi(fileName);
	view->SetFileName(fileName);
	setTabToolTip(addTab(view, view->GetShortFileName()), view->GetFileName());
	setCurrentWidget(view);
	tabBar()->setVisible(count() > 1);
	return view;
}

void FileViewController::DelFileView(FileView * file)
{
	int index = indexOf(file);
	if ( index != -1 )
	{
		removeTab(index);
		delete file;
		tabBar()->setVisible(count() > 1);
	}
}

void FileViewController::ClearAll()
{
	while ( count() )
	{
		QWidget * w =  widget(0);
		removeTab(0);
		delete w;
	}
}

bool FileViewController::IsExist(FileView * view) const
{
	return indexOf(view) != -1;
}



void FileViewController::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void FileViewController::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void FileViewController::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void FileViewController::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls()) 
	{
		QList<QUrl> urlList = mimeData->urls();
		QString text;
		for (int i = 0; i < urlList.size() /*&& i < 32*/; ++i) 
		{
			emit DropWavFile(urlList[i].toLocalFile());
			//break;
		}
		text = text;
    }
	event->acceptProposedAction();
}