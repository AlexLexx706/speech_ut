#include "settingsdialog.h"
#include "../mainwindow.h"
#include <QSettings>
#include <QFontDialog>

SettingsDialog::SettingsDialog(MainWindow * parent)
	: QDialog(parent),
	settings(parent->settings)
{
	ui.setupUi(this);
	connect(this, SIGNAL(accepted()),this, SLOT(OnAccepted()));
	connect(this, SIGNAL(rejected()),this, SLOT(OnRejected()));

	QList<QAction *> list;
	parent->GetAllActions(list);

	ui.tableWidget_actionList->blockSignals(true);
	ui.tableWidget_actionList->clearContents();
	ui.tableWidget_actionList->setRowCount(0);
	foreach (QAction * act, list)
	{
		if ( !act->objectName().isEmpty() )
		{
			QTableWidgetItem * item = new QTableWidgetItem(act->objectName());
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QTableWidgetItem * item2 = new QTableWidgetItem(act->shortcut().toString());
			item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			item2->setData(Qt::UserRole, (int)act);
			ui.tableWidget_actionList->insertRow(ui.tableWidget_actionList->rowCount());
			ui.tableWidget_actionList->setItem(ui.tableWidget_actionList->rowCount()-1, 0, item);
			ui.tableWidget_actionList->setItem(ui.tableWidget_actionList->rowCount()-1, 1, item2);
		}
	}
	QList<QAbstractButton *> bList;
	parent->GetAllButtons(bList);
	foreach (QAbstractButton * button, bList)
	{
		if ( !button->objectName().isEmpty() )
		{
			QTableWidgetItem * item = new QTableWidgetItem(button->objectName());
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QTableWidgetItem * item2 = new QTableWidgetItem(button->shortcut().toString());
			item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			item2->setData(Qt::UserRole, (int)button);
			ui.tableWidget_actionList->insertRow(ui.tableWidget_actionList->rowCount());
			ui.tableWidget_actionList->setItem(ui.tableWidget_actionList->rowCount()-1, 0, item);
			ui.tableWidget_actionList->setItem(ui.tableWidget_actionList->rowCount()-1, 1, item2);
		}
	}
	ui.tableWidget_actionList->resizeColumnsToContents();
	//Установим соответствие с настройками.
	LoadShortCuts();
	ui.tableWidget_actionList->blockSignals(false);
	OnRejected();
}

SettingsDialog::~SettingsDialog()
{}


void SettingsDialog::LoadShortCuts()
{
	//Установим соответствие с настройками.
	settings.beginGroup("ShortCurSettings");
	QStringList keys = settings.childKeys();

	foreach(QString key, keys)
	{
		for (int i = 0; i < ui.tableWidget_actionList->rowCount(); i++ )
		{
			QTableWidgetItem * item = ui.tableWidget_actionList->item(i,1);
			QObject * obj = (QObject *)item->data(Qt::UserRole).toInt();

			if ( obj->objectName() == key )
			{
				if ( QAction * a = qobject_cast<QAction *>(obj) )
				{
					a->setShortcut(settings.value(key).toString());
					item->setText(a->shortcut().toString());
				}
				else if ( QAbstractButton * b = qobject_cast<QAbstractButton *>(obj) )
				{
					b->setShortcut(settings.value(key).toString());
					item->setText(b->shortcut().toString());
				}
			}
		}
	}

	settings.endGroup();
}

void SettingsDialog::on_tableWidget_actionList_itemChanged(QTableWidgetItem * item)
{
	ui.tableWidget_actionList->blockSignals(true);

	settings.beginGroup("ShortCurSettings");
	QObject * obj = (QObject *)item->data(Qt::UserRole).toInt();

	if ( obj )
	{
		QString key;
		if ( QAction * a = qobject_cast<QAction *>(obj) )
		{
			a->setShortcut(item->text());
			key = a->shortcut().toString();
			item->setText(key);
			settings.setValue(a->objectName(), key);
		}
		else if ( QAbstractButton * b = qobject_cast<QAbstractButton *>(obj) )
		{
			b->setShortcut(item->text());
			key = b->shortcut().toString();
			item->setText(key);
			settings.setValue(b->objectName(), key);
		}
		ResetShortcut(obj, key);
	}
	
	settings.endGroup();
	ui.tableWidget_actionList->blockSignals(false);
}

void SettingsDialog::ResetShortcut(const QObject * inObj, const QString & key)
{
	if ( !key.isEmpty() && inObj )
	{
		for (int i = 0; i < ui.tableWidget_actionList->rowCount(); i++ )
		{
			QTableWidgetItem * item = ui.tableWidget_actionList->item(i,1);
			QObject * obj = (QObject *)item->data(Qt::UserRole).toInt();

			if ( obj != inObj &&  item->text() == key  )
			{
				item->setText("");
				settings.setValue(obj->objectName(), "");
			}
		}
	}
}

void SettingsDialog::OnAccepted()
{
	settings.beginGroup("SettingsDialog");

	settings.setValue("copyPhraseTextAC", ui.checkBox_copyPhraseTextAC->isChecked());
	settings.setValue("pauseOffset", ui.checkBox_pauseOffset->isChecked());
	settings.setValue("playAfterChangePhrase", ui.checkBox_playAfterChangePhrase->isChecked());
	settings.setValue("savePhraseText", ui.checkBox_savePhraseText->isChecked());
	settings.setValue("pauseOffsetValue", ui.spinBox_pauseOffset->value());
	settings.setValue("playSpeed", ui.doubleSpinBox_playSpeed->value());
	settings.setValue("autoScalePlot", ui.checkBox_autoScalePlot->isChecked());
	settings.setValue("autoMoveMarker", ui.checkBox_autoMoveMarker->isChecked());
	settings.setValue("playPosTracking", ui.checkBox_playPosTracking->isChecked());
	settings.setValue("checkPhrasesDuraton", ui.checkBox_checkPhrasesDuraton->isChecked());
	settings.setValue("phraseName", ui.lineEdit_phraseName->text());
	settings.setValue("phraseDurationThreshold", ui.spinBox_phraseDurationThreshold->value());
	settings.setValue("editPhraseNameInMarking", ui.checkBox_editPhraseNameInMarking->isChecked());
	settings.setValue("showPluginMassages", ui.checkBox_showPluginMassages->isChecked());
	settings.setValue("openMarkingSameWave", ui.checkBox_openMarkingAsWave->isChecked());
	settings.setValue("openMarkingAsUTF8", ui.checkBox_openMarkingAsUTF8->isChecked());
	settings.setValue("font", font);
	settings.setValue("autoSaveMarking", ui.checkBox_autoSaveMarking->isChecked());
	settings.setValue("autoSaveSignalDesc", ui.checkBox_autoSaveSignalDesc->isChecked());
	settings.setValue("showSignalDescInStartuo", ui.checkBox_show_signal_desc_in_startup->isChecked());

	QApplication::setFont(font);

	settings.setValue("markingFont", markingFont);
	settings.endGroup();
}

void SettingsDialog::OnRejected()
{
	settings.beginGroup("SettingsDialog");

	ui.checkBox_copyPhraseTextAC->setChecked(settings.value("copyPhraseTextAC", true).toBool());
	ui.checkBox_pauseOffset->setChecked(settings.value("pauseOffset", false).toBool());
	ui.checkBox_playAfterChangePhrase->setChecked(settings.value("playAfterChangePhrase", false).toBool());
	ui.checkBox_savePhraseText->setChecked(settings.value("savePhraseText", false).toBool());
	ui.spinBox_pauseOffset->setValue(settings.value("pauseOffsetValue", 0).toInt());
	ui.doubleSpinBox_playSpeed->setValue(settings.value("playSpeed", 1.).toDouble());
	ui.checkBox_autoScalePlot->setChecked(settings.value("autoScalePlot", true).toBool());
	ui.checkBox_autoMoveMarker->setChecked(settings.value("autoMoveMarker", false).toBool());
	ui.checkBox_playPosTracking->setChecked(settings.value("playPosTracking", false).toBool());
	ui.checkBox_checkPhrasesDuraton->setChecked(settings.value("checkPhrasesDuraton", true).toBool());
	ui.lineEdit_phraseName->setText(settings.value("phraseName", "speech").toString());
	ui.spinBox_phraseDurationThreshold->setValue(settings.value("phraseDurationThreshold", 5000).toInt());
	ui.checkBox_editPhraseNameInMarking->setChecked(settings.value("editPhraseNameInMarking", true).toBool());
	ui.checkBox_openMarkingAsWave->setChecked(settings.value("openMarkingSameWave", false).toBool());
	ui.checkBox_openMarkingAsUTF8->setChecked(settings.value("openMarkingAsUTF8", false).toBool());
	font = settings.value("font", QFont()).value<QFont>();
	ui.checkBox_autoSaveMarking->setChecked(settings.value("autoSaveMarking", false).toBool());
	ui.checkBox_autoSaveSignalDesc->setChecked(settings.value("autoSaveSignalDesc", false).toBool());
	ui.checkBox_show_signal_desc_in_startup->setChecked(settings.value("showSignalDescInStartuo", true).toBool());
	
	QApplication::setFont(font);

	markingFont = settings.value("markingFont", QFont()).value<QFont>();
	settings.endGroup();
}

void SettingsDialog::on_pushButton_setFont_clicked()
{
	bool ok;
	QFont f = QFontDialog::getFont(&ok, font);
	if ( ok )
		font = f;
}

void SettingsDialog::on_pushButton_setMarkingFont_clicked()
{
	bool ok;
	QFont f = QFontDialog::getFont(&ok, markingFont);
	if ( ok )
		markingFont = f;
}