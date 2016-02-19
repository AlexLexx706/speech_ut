#include "fileview.h"
#include "../waveviewerwidget/frame/dataframe/dataframe.h"
#include "../generatedfiles/ui_fileview.h"
#include "../fileviewcontroller/fileviewcontroller.h"

#define ui (*((Ui::FileViewClass *)__ui))

FileView::FileView(QWidget *parent)
	: QFrame(parent),
	__ui(new Ui::FileViewClass())
{
	ui.setupUi(this);
	ui.phraseController->SetButtonController(&ui.waveWidget->GetButtonController());
	ui.waveWidget->SetPhraseController(ui.phraseController);
	ui.phraseController->SetWindow(ui.waveWidget->GetWindow());

	connect(ui.phraseController, SIGNAL(PhraseDataChanged()), ui.waveWidget, SLOT(update()));
	connect(ui.waveWidget, SIGNAL(WindowChanged(const QRectF &)), ui.phraseController, SLOT(SetWindow(const QRectF &)));
	connect(ui.waveWidget->GetWaveData(), SIGNAL(DataChanged(long)), ui.phraseController, SLOT(SetMax(long)));
	connect(ui.phraseController, SIGNAL(ScaleHRuller(qreal, int)), ui.waveWidget, SLOT(ScaleHRuller(qreal, int)));

	connect(ui.waveWidget, SIGNAL(SelectedSamplesChanged(long,long)), this, SIGNAL(SelectedSamplesChanged(long, long)) );
	connect(ui.waveWidget, SIGNAL(PosChanged(long, double)), this, SIGNAL(PosChanged(long, double)) );
	connect(ui.waveWidget, SIGNAL(DropFile(const QString &)), this, SIGNAL(DropWavFile(const QString &)) );
	connect(ui.waveWidget, SIGNAL(DataChanged(long)), this, SIGNAL(DataChanged(long)));

	connect(ui.phraseController, SIGNAL(CurrentPhraseChanged()), this, SIGNAL(CurrentPhraseChanged()) );
	connect(ui.phraseController, SIGNAL(PhraseDataChanged()), this, SIGNAL(PhraseDataChanged()) );
	connect(ui.phraseController, SIGNAL(CurrentListChanged()), this, SIGNAL(CurrentListChanged()) );
	connect(ui.phraseController, SIGNAL(ListDataChanged()), this, SIGNAL(ListDataChanged()) );
	connect(ui.phraseController, SIGNAL(PhraseDoubleClick()), this, SIGNAL(PhraseDoubleClick()) );
	connect(ui.phraseController, SIGNAL(DropFile(const QString & )), this, SIGNAL(DropMarkingFile(const QString & )) );
}

void FileView::setPhraseControllerUp(bool up)
{
	if ( up )
		ui.splitter->insertWidget(0, ui.phraseController);
	else
		ui.splitter->insertWidget(0, ui.waveWidget);
}

void FileView::SetFileName(const QString & _fileName)
{
	fileName = _fileName;
	emit FieNameChanged(fileName);
}


FileView::~FileView()
{
	delete (Ui::FileViewClass *)__ui;
}

PhraseController * FileView::GetPhraseController() const {return ui.phraseController;}

WaveViewerWidget * FileView::GetWaveWidget() const {return ui.waveWidget;}