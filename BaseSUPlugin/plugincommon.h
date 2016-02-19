#ifndef PLUGIN_COMMON_H
#define PLUGIN_COMMON_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QPair>
#include <QDataStream>

class QSettings;
class QMenu;
class QToolBar;


//! ������ ��������.
struct SUPPhraseData
{
	//!�������� � ��������
	qint32 Start;
	//!�������� � ��������
	qint32 End;	
	//! ��� �����
	QString Name;
	
	//! �����������.
	SUPPhraseData(): Start(0), End(0){}

	//! �����������.
	SUPPhraseData(const SUPPhraseData & data)
	{
		Start = data.Start;
		End = data.End;
		Name = data.Name;
	}

	//! �����������.
	SUPPhraseData(qint32 _Start, qint32 _End, QString _Name):Start(_Start),End(_End),Name(_Name){}

	//! �������� = .
	SUPPhraseData & operator = (const SUPPhraseData & data )
	{
		Start = data.Start;
		End = data.End;
		Name = data.Name;
		return *this;
	}
};

//! ������� ������ ��� ������� GetMarking
struct SUPInputData
{
	//! ��� �� ������.
	int BitsPerSample;
	
	//! ������� �������������.
	int SampleRate;

	//! ������ �������.
	QByteArray Signal;

	//! ��������� � �������, ��� ������� ������������ ��������.
	QList< QPair<qint32, qint32> > Intervals;

	//! ��������� �������. � ����������� �������� �� ������������, �� ���� ������������, �� ��������� ������ � ������� JSON.
	QString PluginSettings;
};

//! �������� ������ ��� ������� GetMarking
struct SUPOutData
{
	//! ������ ��������.
	QList<SUPPhraseData> Marking;
	
	//! �������������� ������ ����������� ���������(������ ��� JSON ������.).
	QString AdditionData;
};

//! ��������� ��������� �����, ��� �������� ���������.
struct SUPFileResult
{
	QString FileName;
	QString ErrorDesc;
	bool Result;
};



//��������� ������� ��� speech_utility.
class SUPluginInterface
{
public:

	//! �������� �������������� ��������.
	enum ActionsType
	{
		//! �������� ��������.
		MakeMarking = 0x01,

		//! �������� ���������.
		BatchProcessing = 0x02,
	};

	//! ����������.
	virtual ~SUPluginInterface(){}

	//! ������������� �������.
	virtual bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar) = 0;

	//! ���������� ������ � ��������
	virtual bool Free() = 0;

	//! ��������� �������.
	virtual bool IsInit() const = 0;

	//! �������� ��� �������.
	virtual QString GetPluginName() const  = 0;

	//! �������� �������� �������������� ��������.
	virtual int GetActionsTypes() const  = 0;

	//! �������� ������ ���������.
	virtual void OpenSettingsDialog() = 0;

	/** 
	 * ��������� �������� ��������, �� ����������� �������, ������ ��������� ����� �������� ��������.
	 * ��������� ���������� � ������� ������������� � �������.
	*/
	virtual bool StartMakeMarking(int ID, const SUPInputData & inData) = 0;

	/** 
	 * ��������� �������� ��������� ������, �� ����������� �������, ������ ��������� ����� �������� ��������.
	 * ��������� ���������� � ������� ������������� � �������.
	*/
	virtual bool StartBatchProcessing(int ID, const QString & settings) = 0;
	
	//! ���������� �������� ��������� ������.
	virtual bool StopBatchProcessing(int ID) = 0;

	//! �������� �������� ��������� ������.
	virtual QString GetLastErrorDesc() const = 0;

	//! �������� ������ ����������� ����� �������� ���������.
	virtual void GetBatchResult(QList<SUPFileResult> & result) = 0;

	//! �������� ��������� �������.
	virtual QString GetPluginSettings() const = 0;

};

Q_DECLARE_INTERFACE(SUPluginInterface, "{05AF5C94-26E8-4410-B51C-3BE0E05EB7CD}");

#endif // PLUGIN_COMMON_H
