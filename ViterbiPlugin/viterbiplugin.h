#ifndef VITERBIPLUGIN_H
#define VITERBIPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <Viterbi/Engine.h>
//#include <Viterbi/definesGlobal.h>
#include <Preprocessing/ppcEngine.h>
#include "../json_spirit/json_spirit.h"



//! ���������� ������ �������.
class ViterbiPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	ViterbiPlugin();

	//! ����������.
	~ViterbiPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const { return isInit; };

	//! �������� ��� �������.
	QString GetPluginName() const { return "Plugin_PhonemeRecognition"; }

	//! �������� �������� �������������� ��������.
	int GetActionsTypes() const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing; }

	//! �������� ������ ���������.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec(); }

	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "ViterbiPlugin.dll";}

protected:
	//! ������� ���������� �������� �������������� ��������.
	virtual bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	/** 
	 * ���������� ����� ������� ��������� ���������, ������������� ��������
	 * @param id
	 * @param files_reports [out] - ������� ������ ��������� ������ QPair<���� �� ���������, ���� ������>
	 * @param errorDesc [out] - �������� ������.
	 * @result - ���������.
	 */
	bool InitBatchProcessing(void * & internalData, int id,  const QString & settingsStr, QString & errorDesc);

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
		QString				ModelFileName;
		PRUNING_T 	PruningType;
		SP_REAL				PruningPar;
		LOOPACTIONS_T LoopActionsType;
		bool				UseLangModel;
		int					NGramSize;

		NBEST_T		NBestType;
		int					NBestCount;
		int					NBestCapacity;
		bool				PrintLatticeOutput;
		QString				LatticeOutputFile;
		bool				UseLatticeOutputDir;
		QString				LatticeOutputDir;
		json_spirit::mObject plp;
	};

	//! ������������ ��� �������� ������ ������ ��������� �����.
	struct BatchThreadData
	{
		MoreEngine batchRecEngine;
		Settings settings;
	};

	QMenu * menu;
	SettingsDialog * dialog;
	bool isInit;
	ppcEngine ppc;

	void AddMarking(int sampleRate, 
					long offset, 
					const std::vector<std::list<std::pair<int, std::string>>> & inResult, 
					QList<SUPPhraseData> & marking);
	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);

	//! ������� �����.
	bool ViterbiPlugin::ComputePhonemes(MoreEngine & recEngine, 
										 const std::vector<short>  & signal, 
										 const Settings & settings,
										 const char * fileName,
										 std::vector<std::list<std::pair<int, std::string>>> & outputPath,
										 QString & lastErrorDesc);

};

#endif // VITERBIPLUGIN_H
