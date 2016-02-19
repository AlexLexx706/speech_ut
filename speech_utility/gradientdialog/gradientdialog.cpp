#include "gradientdialog.h"

GradientDialog::GradientDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.colorDialog->show();
	ui.colorDialog->setParent(this, Qt::Widget);
}

GradientDialog::~GradientDialog()
{

}
