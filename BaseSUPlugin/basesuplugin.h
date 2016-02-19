#ifndef BASESUPLUGIN_H
#define BASESUPLUGIN_H

#include <QObject>
#include "basesuplugin_global.h"
#include "plugincommon.h"
#include <QFile>
#include <QTextStream>

class InternalThread;
class QDialog;
class QMutex;
class QAction;

//! ���������� ������ �������.
/**
	������ ����������� �������� ������� � ������� JSON, ������������ � �������: 
		StartMakeMarking, StartBatchProcessing:
		{
			pluginName:"xxx",
			parameters:{
				reportsDir:"",
				processingDir:"",
				maxBPThreadCount:2
				useExternalProcessing: true
			}
		}
		pluginName - ��� �������.
		parameters - ��������� �������.
		reportsDir - ��� ����� ��� ���������� ��������.
		processingDir - ��� ����� � ������� �� ���������.
		maxBPThreadCount - ������������ ���������� �������.
		useExternalProcessing - ������������ ������ �� ������� �������.
*/

class BASESUPLUGIN_EXPORT BaseSUPlugin:public QObject, public SUPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(SUPluginInterface)
public:
	//! �����������.
	BaseSUPlugin();

	//! ����������.
	~BaseSUPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar) { return false; }

	//! ���������� ������ � ��������
	bool Free() { return false; }

	//! ��������� �������.
	bool IsInit() const { return false; }

	//! �������� ��� �������.
	QString GetPluginName() const {return "Base plugin";}

	//! �������� �������� �������������� ��������.
	int GetActionsTypes()const {return MakeMarking;}

	QString GetLastErrorDesc() const { return lastErrorDesc; }

	//! �������� ������ ����������� ����� �������� ���������.
	void GetBatchResult(QList<SUPFileResult> & result);

	//! �������� ����� ��������� �������� ���������.
	bool GetCommonBatchResult();

	//! �������� ��������� �������.
	QString GetPluginSettings() const {return "";}

	//! �������� ��� dll �������.
	virtual QString GetLibName() const = 0;

	virtual QString GetBatchFileProcessngMask() { return "*.wav";};
	virtual QString GetBatchFileResultMask() { return ".lab";}

public slots:
	//! �������� ������ ���������.
	void OpenSettingsDialog(){}

	//! ��������� �������� ��������, �� ����������� �������, ������ ��������� ����� �������� ��������. 
	bool StartMakeMarking(int ID, const SUPInputData & inData);

	//! ��������� �������� ��������� ������.
	bool StartBatchProcessing(int ID, const QString & settings);
	
	//! ���������� �������� ��������� ������.
	bool StopBatchProcessing(int ID);

signals:
	//! �������� �������� ������.�� ���������
	void MakeMarkingStarted(int ID);

	//! �������� �������� ���������.
	void MakeMarkingFinished(int ID, const SUPOutData & data, bool result, const QString & errorDesc);

	//! ������ �������� ���������.
	void BatchProcessingStarted(int ID);

	//! �������� ��������� ���������.
	void BatchProcessingFinished(int ID, bool result, const QString & errorDesc);

	//! �������� �������� ���������.
	void ProgressMessage(int id, qreal progress, const QString & message);

	//! ������ ��������� �����.
	void StartFileProcessing(int id, const QString & fileName);

	//! ��������� ��������� �����.
	void FileProcessingResult(int id, const SUPFileResult & result);

	//! ������ �������� ��������.
	void GetMarkingTriggered();

	//! ������ �������� ��������.
	void GetMarkingOnMarkingTriggered();

	//! ������ ��������� �������� ���������.
	void StartBatchProcessingTriggered();

	//! ������ ���������� �������� ���������.
	void StopBatchProcessingTriggered();

protected:
	friend class InternalThread;
	friend class BatchThread;

	//! �������� ������.
	QString lastErrorDesc;

	//! ������� ���������� �������� �������������� ��������.
	virtual bool GetMarking(int id, const SUPInputData & inData, SUPOutData & outData, QString & errorDesc);

	//! ������� ���������� ����� ������ ������� MakeMarkingFinished, ��������� ������� ��������� � MakeMarkingFinished.
	virtual bool PostProcessResult(int ID, const SUPOutData & data, bool result, QString & errorDesc);

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
	 * ���������� ��� ������� �����.
	 * @param id [in] - id ��������� � ������ �����������.
	 * @param fileDesc [in] - �������� QPair<��� �����, ��� ����� ������>, ���� ������ ������ ���� ������ � ������ �������.
	 * @param errorDesc [out] - �������� ������.
	*/
	virtual bool FileProcessing(void * internalData, int id, const QPair<QString, QString> & fileDesc, QString & errorDesc);

	/** 
	 * ���������� ��� ������� ������ ����� ���������� ��������� ������.
	 * @param id [in] - ID
	*/
	virtual void ReleaseBatchProcessing(void * & internalData, int id);


	//! ��������� �������� � ����.
	static bool SaveMarkingInFile(const QString & fileName, const QList<SUPPhraseData> & marking, QString & errorDesc);

	//! ��������� ������ �� wav ����� ������������� �������.
	static bool LoadFile(const QString & fileName, int bitPerSample, int samplerate, QByteArray & signal, QString & errorDesc);

	//! �������� ������ � ����.
	static bool SaveFile(const QString & fileName, int cannels, int bitPerSample, int samplerate, QByteArray & signal, QString & errorDesc);

	QMenu * InitActions(QMenu * inMenu, QToolBar * inToolBar, unsigned int actionsMask = 0x1f);

	//������������� ����� �������.
	virtual void RenameActionsNames(QList< QAction *> & actions);

private:
	QList<InternalThread *> threads;

	InternalThread * batchProcessingThread;

	QList<SUPFileResult> batchResult;

	QMutex * batchResultMutex;

	QFile completedFiles;
	QTextStream completedFilesStream;

	QFile errorCompletedFiles;
	QTextStream errorCompletedFilesStream;

	InternalThread * GetFreeThread();

	void EmitStartSignal(int ID, SUPluginInterface::ActionsType type);

	void FileProcessingInternal(void * internalData, int id, const QPair<QString, QString> & file_report);

	void EmitFileProcessingResult(int id, const SUPFileResult & res);

	void EmitProgressMessage(int id, qreal progress, const QString & message);

	void EmitStartFileProcessing(int id, const QString & fileName);

	//! ���������� ������������ ������.
	int GetCompletedFilesCount() const;

	//! ����� ���������� �������������� ������.
	int GetFilesCount() const;

	//! �������� ����� ��������� ��� ��������� ���������.
	bool GetCommonBatchSettings(const QString & settings,
										QList< QPair<QString, QString> > & files,
										int & maxBPThreadCount,
										QString & errorDesc);
	
	//! ������������ ������� ���������, � ��������� ��������.
	bool IsUseExternalProcessing(const QString & settings, QString & errorDesc);

	//! ���������� �������� ��� ��������� ����������.
	bool PrepareForExernalProcessing(const QString & settings, QString & outSettings, QString & errorDesc);

private slots:
	void OnThreadFinished();
};

//! ���������� ��� ������������.
BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPPhraseData & data);

//! ���������� ��� ������������.
BASESUPLUGIN_EXPORT QDataStream & operator >>(QDataStream & out, SUPPhraseData & data);

BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPInputData & data);

BASESUPLUGIN_EXPORT QDataStream & operator >> (QDataStream & out, SUPInputData & data);

BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPOutData & data);

BASESUPLUGIN_EXPORT QDataStream & operator >> (QDataStream & out, SUPOutData & data);

BASESUPLUGIN_EXPORT QDataStream & operator << (QDataStream & out, const SUPFileResult & data);

BASESUPLUGIN_EXPORT QDataStream & operator >> (QDataStream & out, SUPFileResult & data);


#endif // BASESUPLUGIN_H
