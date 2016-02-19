#include "aboutdialog.h"
#include "../Version.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.label_version->setText(QString::number(progBuildVersion));
}

AboutDialog::~AboutDialog()
{

}
