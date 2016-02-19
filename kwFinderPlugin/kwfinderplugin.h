#ifndef KWFINDERPLUGIN_H
#define KWFINDERPLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <Viterbi/Engine.h>



//! ���������� ������ �������.
class KWFinderPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	KWFinderPlugin();

	~KWFinderPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const { return isInit; }

	//! �������� ��� �������.
	QString GetPluginName() const {return "Plugin_WordRecognition";};

	//! �������� �������� �������������� ��������.
	int GetActionsTypes()const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing;};

	//! �������� ������ ���������.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec();}
	
	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "kwFinderPlugin.dll";}

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
		QString PathToHmm;
		QString PathToDict;
		bool dictInUtf8;
		QString PathToTiedls;

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
	};
	
	//! ������������ ��� �������� ������ ������ ��������� ������.
	struct BatchThreadData
	{
		MoreEngine * batchRecEngine;
		Settings settings;
	};


	SettingsDialog * dialog;
	QMenu * menu;
	bool isInit;

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);	
	
	
	static bool AddMarking(int sampleRate,
							int offset, 
							bool useUtf8,
							const std::vector<std::list<std::pair<int, std::string> > > & result, 
							QList<SUPPhraseData> & marking, 
							QString & lastErrorDesc);

	static bool FindWords(MoreEngine & engine, 
						  const Settings & settings,
						  const char * latticeFileName,
						  const short* signal, 
						  int signalSize,
						  std::vector<std::list<std::pair<int, std::string> > > & outputPath,
						  QString & errorDesc);
	static bool InitEngine(MoreEngine & engine, const Settings & settings, 
							QString & lastErrorDesc);
};


#endif // KWFINDERPLUGIN_H
