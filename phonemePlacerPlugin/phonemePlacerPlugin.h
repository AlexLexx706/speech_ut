#ifndef CNDVITERBIPLUGIN_H
#define CNDVITERBIPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <QMutex>

//! ���������� ������ �������.
class PhonemePlacerPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	PhonemePlacerPlugin();

	//! ����������.
	~PhonemePlacerPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const { return isInit; }

	//! �������� ��� �������.
	QString GetPluginName() const {return "Plugin_PhonemePlacer";}

	//! �������� �������� �������������� ��������.
	int GetActionsTypes()const {return MakeMarking | BatchProcessing;}

	//! �������� ������ ���������.
	void OpenSettingsDialog(){if (dialog) dialog->exec();}

	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "PhonemePlacerPlugin.dll";}

protected:
	//! ������� ���������� �������� �������������� ��������.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! ������� ���������� ����� ������ ������� MakeMarkingFinished, ��������� ������� ��������� � MakeMarkingFinished.
	bool PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc);

	/**
	 * ������ �������� ���������.
	 * �������� � ������ �������� ��������� � ��� ������, ������������ ��� ������������� ����� ��������.
	 * @param id [in] - �������� ��������� �������������.
	 * @param settings [in] - ���������.
	 * @param errorDesc [out] - �������� ������.
	*/
	virtual bool BeginBatchProcessing(int id, const QString & settings, QString & errorDesc);

	/**
	 * ���������� �������� ���������.
	 * �������� � ������ �������� ��������� � ��� �����, ������������ ��� ������������ ����� ��������.
	 * @param id [in] - �������� ��������� �������������.
	*/
	virtual void EndBatchProcessing(int id);

	/** 
	 * ���������� ��� ������� ������, ��� ��� ������.
	 * @param id
	 * @param files_reports [out] - ������� ������ ��������� ������ QPair<���� �� ���������, ���� ������>
	 * @param errorDesc [out] - �������� ������.
	 * @result - ���������.
	 */
	virtual bool InitBatchProcessing(void * & internalData, int id, const QString & settings, QString & errorDesc);

	/** 
	 * ���������� ��� ������� ������ ����� ���������� ��������� ������.
	 * @param id [in] - ID
	*/
	virtual void ReleaseBatchProcessing(void * & internalData, int id);


	/**
	 * ���������� ��� ������� �����.
	 * @param id [in] - id ��������� � ������ �����������.
	 * @param fileDesc [in] - �������� QPair<��� �����, ��� ����� ������>, ���� ������ ������ ���� ������ � ������ �������.
	 * @param errorDesc [out] - �������� ������.
	*/
	virtual bool FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc);

	//������������� ����� �������.
	virtual void RenameActionsNames(QList< QAction *> & actions);

private:
	struct Settings
	{
		QString HmmModel;
		QString TextFileName;
		bool	PhraseIsCorrect;
		QString TranscriptionsDir;
		bool is_output_type_by_states;
	};

	//! ������������ ��� �������� ������ ������ ��������� �����.
	struct BatchThreadData
	{
		Settings settings;
		int placer_id;
	};

	SettingsDialog * dialog;
	QMenu * menu;
	bool isInit;

	QMap<int, int> model_map;
	QMutex model_map_locker;

	//! �������� ������������ �� �����.
	bool LoadTranscription(const QString & fileName, std::string & transcription, QString & lastErrorDesc);

	//! ������� ������ ����������� �������������� ���������.
	QString CreateAdditionResult(bool phraseIsCorrect);

	//! �������� ��������.
	void AddMarking(long framesCount, const std::list<std::pair<std::string, int> > & inResult, long offset, QList<SUPPhraseData> & marking);

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
	
	bool GetModelId(int id, int &model_id, bool remove_id);
};

#endif // CNDVITERBIPLUGIN_H
