#ifndef GRADIENTDIALOG_H
#define GRADIENTDIALOG_H

#include <QDialog>
#include "ui_gradientdialog.h"

class GradientDialog : public QDialog
{
	Q_OBJECT

public:
	GradientDialog(QWidget *parent = 0);
	~GradientDialog();

private:
	Ui::GradientDialogClass ui;
};

#endif // GRADIENTDIALOG_H
