#ifndef VADMARKINGPLUGIN_H
#define VADMARKINGPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include <QLibrary>
#include "settingsdialog/settingsdialog.h"
#include <Speaker Identification\SDiSeCEngine.h>
#include <Models Processing\GMM\eGmmTk.h>

//! ���������� ������ �������.
class SpeakersCountPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	SpeakersCountPlugin();

	//! ����������.
	~SpeakersCountPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const {return isInit;}

	//! �������� ��� �������.
	QString GetPluginName() const{return "SpeakersCount_VAD";}

	//! �������� �������� �������������� ��������.
	int GetActionsTypes() const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing; }

	//! �������� ������ ���������.
	void OpenSettingsDialog(){ if ( dialog) dialog->exec();}
	
	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "VadMarkingPlugin.dll";}


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
	bool InitBatchProcessing(void * & internalData, int id,  const QString & settings, QString & errorDesc);

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
		QString ubmModelPath;
		QString speechModelPath;
		QString pauseModelPath;
		double tresholdParam;
	};

	struct ThreadData
	{
		eGmmTk gmmTk;
		SP_BYTE* UBM;
		SP_BYTE* speechModel;
		SP_BYTE* nonSpeechModel;
		Settings settings;
		ThreadData():UBM(NULL),speechModel(NULL), nonSpeechModel(NULL){}
	};

	QMenu * menu;
	SettingsDialog * dialog;
	bool isInit;

	//! �������� �������.
	bool doVAD(const QByteArray & signal, QList<SUPPhraseData> & marks);

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
};

#endif // VADMARKINGPLUGIN_H
