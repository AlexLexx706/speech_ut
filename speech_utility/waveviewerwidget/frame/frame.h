#ifndef FRAME_A_H
#define FRAME_A_H

#include <QObject>
#include <QRectF>
#include <QPixmap>
#include "../../speech_utility_global.h"

//! Класс формирует рисунок.
class SPEECH_UTILITY_EXPORT Frame:public  QObject
{
	Q_OBJECT
public:
	Frame();
	
	//! Установить окно.
	virtual void SetWindow(const QRectF & window);
	
	//!Окно.
	inline QRectF GetWindow() const {return window;}
	
	//! Установить размеры.
	virtual void SetSize(const QSize & size);

	inline QSize GetSize() const {return pixmap.size();}

	//! Получить рисунок..
	inline QPixmap GetPixmap() const {return pixmap;}

	//! рисунок подготовлен.
	inline bool IsReady() const {return isReady;}
signals:
	void ImageUpdated();

	void WindowChanged(const QRectF & window);

protected:
	QPixmap pixmap;
	QRectF window;
	bool isReady;

protected slots:
	//! Обновить рисунок.
	virtual void Update() = 0;
};

#endif // FRAME_A_H
