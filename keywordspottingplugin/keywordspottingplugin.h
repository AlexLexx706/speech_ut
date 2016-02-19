#ifndef KeywordSpottingPlugin_H
#define KeywordSpottingPlugin_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"
#include <Viterbi/Engine.h>




//! ���������� ������ �������.
class KeywordSpottingPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	KeywordSpottingPlugin();

	~KeywordSpottingPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const { return isInit; }

	//! �������� ��� �������.
	QString GetPluginName() const {return "Plugin_KeywordSpotting";};

	//! �������� �������� �������������� ��������.
	int GetActionsTypes()const{return SUPluginInterface::MakeMarking  | SUPluginInterface::BatchProcessing;};

	//! �������� ������ ���������.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec();}

	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "keywordSpottingPlugin.dll";}

protected:
	//! ������� ���������� �������� �������������� ��������.
	bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! ������ �������� ���������.
	virtual bool BeginBatchProcessing(int id, const QString & settings, QString & errorDesc);

	//! ���������� �������� ���������.
	virtual void EndBatchProcessing(int id);


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
	//! ���������.
	struct Settings
	{
		QString PathToHmm;
		QString PathToDict;
		bool dictInUtf8;
		QString PathToTiedls;
		QString PathToKeywordsDict;
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
	QString commonDictFile;
	QStringList keywords;



	//! ��������� ������ � base64
	std::string base64_encode(const unsigned char * buffer, unsigned int len)
	{
		QByteArray temp((const char *)buffer, len);
		return QString(temp.toBase64()).toStdString();	
	}
	static bool AddMarking(int sampleRate, 
								int offset,
								bool dictInUtf8,
								 const std::vector<std::list<std::pair<int, std::string> > > & _result, 
								 const QStringList & keywords,
								 QList<SUPPhraseData> & marking, 
								 QString & lastErrorDesc);

	static bool CreateCommonDict(const QString & dict,
								const QString & keywordsDict, 
								bool dictInUtf8,
								QString & outCommonDict,
								QStringList & outKeywords,
								QString & outErrorDesc);

	static bool IsWordsSimilarity(const QString & w1, const QStringList & w2);

	static bool InitEngine(MoreEngine & engine, const Settings & settings, QString & commonDictFile, QString & lastErrorDesc);

	static bool FindWords(MoreEngine & engine, 
						  const short* signal, 
						  int signalSize, 
						  std::vector<std::list<std::pair<int, std::string> > > & outputPath,
						  QString & errorDesc);

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
};


#endif // KeywordSpottingPlugin_H
