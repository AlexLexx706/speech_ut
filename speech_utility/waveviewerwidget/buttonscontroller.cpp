#include "buttonscontroller.h"
#include "waveviewerwidget.h"
#include <iostream>
#include <assert.h>
#include <math.h>
#include <QMouseEvent>

Button::Button(ButtonsController * _controller):
	controller(_controller),
	visible(true)
{
	if ( controller )
		controller->AddButton(this);
}

Button::~Button()
{
	if ( controller )
		controller->RemoveButton(this);
}

ButtonsController::ButtonsController(WaveViewerWidget & _widget):
	widget(_widget),
	currentButton(NULL)
{

}

ButtonsController::~ButtonsController()
{
	QList< Button * > temp = buttons;
	foreach(Button * button, temp)
	{
		delete button;
	}
}

bool ButtonsController::AddButton(Button * button)
{
	if ( button && buttons.indexOf(button) == -1 )
	{
		buttons.append(button);
		return true;
	}
	return false;
}

bool ButtonsController::RemoveButton(Button * button)
{
	int index = buttons.indexOf(button);
	if ( index != -1 )
	{
		buttons.takeAt(index);
		if ( currentButton == button )
			currentButton = NULL;

		return true;
	}
	return false;

}

Button * ButtonsController::SelectAndPressButton(QMouseEvent * event)
{
	//Выбираем контрол 
	if ( buttons.size() )
	{
		int index = currentButton ? (buttons.indexOf(currentButton) + 1) % buttons.size() : 0;
		int count = 0;

		while ( count < buttons.size() )
		{
			QRect rect = buttons[index]->GetRect();
			if ( rect.contains(event->pos()) )
			{
				currentButton = buttons[index];
				currentButton->MousePress(event);
				return currentButton;
			}
			count++;
			index++;
			index = index % buttons.size();
		}
	}
	return NULL;
}

void ButtonsController::ReleaseButton(QMouseEvent * event)
{
	if ( currentButton )
		currentButton->MouseRelease(event);
}

//! Получить кнопку.
Button * ButtonsController::GetButton(const QPoint & pos)
{
	foreach(Button * button, buttons)
	{
		if ( button->GetRect().contains(pos) )
			return button;
	}
	return NULL;
}

Button * ButtonsController::GetCurrentButton()
{
	return currentButton;
}

const QList< Button * > & ButtonsController::GetData() const
{
	return buttons;
}

WaveViewerWidget & ButtonsController::GetWidget() const
{
	return widget;
}