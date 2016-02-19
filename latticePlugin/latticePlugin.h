#ifndef LATTICE_PLUGIN_H
#define LATTICE_PLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"


//! ���������� ������ �������.
class latticePlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	latticePlugin();

	//! ����������.
	~latticePlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const { return isInit; }

	//! �������� ��� �������.
	QString GetPluginName() const {return "latticePlugin";}

	//! �������� �������� �������������� ��������.
	int GetActionsTypes()const {return BatchProcessing;}

	//! �������� ������ ���������.
	void OpenSettingsDialog(){if (dialog) dialog->exec();}

	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "latticePlugin.dll";}

	QString GetBatchFileProcessngMask() { return "*.lat";};

protected:
	//! ������� ���������� �������� �������������� ��������.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	/** 
	 * ���������� ����� ������� ��������� ���������, ������������� ��������
	 * @param id
	 * @param files_reports [out] - ������� ������ ��������� ������ QPair<���� �� ���������, ���� ������>
	 * @param errorDesc [out] - �������� ������.
	 * @result - ���������.
	 */
	bool InitBatchProcessing(void * & internalData, int id, const QString & settingsStr, QString & errorDesc);

	/**
	 * ���������� ��� ������� �����.
	 * @param id [in] - id ��������� � ������ �����������.
	 * @param fileDesc [in] - �������� QPair<��� �����, ��� ����� ������>, ���� ������ ������ ���� ������ � ������ �������.
	 * @param errorDesc [out] - �������� ������.
	*/
	bool FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc);

	/** 
	 * ���������� ����� ���������� ���������, ������������ ��������.
	 * @param id [in] - ID
	*/
	void ReleaseBatchProcessing(void * & internalData, int id);

	//������������� ����� �������.
	virtual void RenameActionsNames(QList< QAction *> & actions);

private:
	struct Settings
	{
		QString programmFileName;
		QMap< QString, QString > keys;
		int StartThreshold;
		int EndThreshold;
		bool ShowBest;
		QString latticeFile;
		bool UsePostProcessing;
	};

	SettingsDialog * dialog;
	QMenu * menu;
	QString transcriptionsDir;
	bool isInit;
	Settings settings;


	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);

	//! �������� �������� �� �����.
	bool GetMarking(const QString & fileName, 
					QList<SUPPhraseData> & Marking, 
					QString & errorDesc, 
					bool showBest,
					int startThreshold, 
					int endThreshold);


	//! �������� �������
	static void GetBestPhrase(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * phrase,
					   QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > > & list,
					   qlonglong Start_Threshold, 
					   qlonglong End_Threshold,
					   QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > & best);

	static bool phrase_in_best(QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * phrase, 
						QList< QPair< QPair<qlonglong, qlonglong>, QPair<QString, qreal> > * > & list,
						qlonglong Start_Threshold,
						qlonglong End_Threshold );


};

#endif // LATTICE_PLUGIN_H
