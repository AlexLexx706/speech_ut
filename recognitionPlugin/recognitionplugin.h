#ifndef RECOGNITION_PLUGIN_H
#define RECOGNITION_PLUGIN_H

#include "../BaseSUPlugin/basesuplugin.h"
#include "settingsdialog/settingsdialog.h"

using namespace MoRe;

//! ���������� ������� ������������� ������.
class RecognitionPlugin:public BaseSUPlugin
{
	Q_OBJECT
public:
	//! �����������.
	RecognitionPlugin();

	//! ����������.
	~RecognitionPlugin();

	//! ������������� �������.
	bool Init(QSettings * inSetting, QMenu * menu, QToolBar * toolBar);

	//! ���������� ������ � ��������
	bool Free();

	//! ��������� �������.
	bool IsInit() const { return isInit; };

	//! �������� ��� �������.
	QString GetPluginName() const { return QString::fromLocal8Bit("������������� ������."); }

	//! �������� �������� �������������� ��������.
	int GetActionsTypes() const{return SUPluginInterface::BatchProcessing; }

	//! �������� ������ ���������.
	void OpenSettingsDialog(){ if ( dialog ) dialog->exec(); }

	//! �������� ��������� �������.
	QString GetPluginSettings() const;

	//! �������� ��� dll �������.
	QString GetLibName() const {return "RecognitionPlugin.dll";}

protected:
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
		QString PauseModel;
		QString SpeechModel;
		QString NoiceModels;
		QString LanguagesModels;
		QString PhonemModels;
	};

	//! ������������ ��� �������� ������ ������ ��������� �����.
	struct BatchThreadData
	{
		Settings settings;
	};

	QMenu * menu;
	SettingsDialog * dialog;
	bool isInit;

	bool GetSettings(const QString & str, Settings & settings, QString & errorDesc);
};

#endif // RECOGNITION_PLUGIN_H
