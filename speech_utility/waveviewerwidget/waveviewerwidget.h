#ifndef WAVEVIEWER1_H
#define WAVEVIEWER1_H

#include <QFrame>
#include <QVector>
#include <QPair>
#include <QRectF>
#include "../speech_utility_global.h"

class QPainter;
class PhraseController;
class ButtonsController;
class ScrollButton;
class SelectButton;
class Button;
class QTimer;
class VRullerFrame;
class HRullerFrame;
class DataFrame;
class QMenu;

class SPEECH_UTILITY_EXPORT WaveViewerWidget : public QFrame
{
	Q_OBJECT

public:
	friend class WaveData;

	//! Конструктор.
	WaveViewerWidget(QWidget *parent);
	
	//! Деструктор.
	~WaveViewerWidget();

	//! Установить окно отображения.
	bool SetWindow(const QRectF & window);

	//! Получить окно отображения.
	const QRectF & GetWindow() const;

	//! Получить окно полностью описывающее данные.
	const QRectF & GetMaxWindow() const;

	//! Размер минимального окна.
	const QSizeF & GetMinWindowSize() const;

	//! Получить выделенное окно.
	QRectF GetSelectedWindow();

	//! Разрешить выделение окна.
	void EnableSelectWindow(bool enable);

	//! Разрешить выделение окна.
	bool IsEnableSelectWindow();

	//! Разрешить перемещение и масштабирование.
	void EnableTranslate(bool enable);

	//! Разрешить перемещение и масштабирование.
	bool IsEnableTranslate() const;

	//! Разрешить выделять семплы.
	void EnableSelectSamples(bool enable);

	//! Разрешить выделять семплы.
	bool IsEnableSelectSamples() const;

	//! Преобразует координаты из координат окна в логические.
	QPointF Transform(const QPoint & pos, bool offset = false);

	//! Преобразует координаты из логических в координаты окна.
	QPoint InvTansform(const QPointF & pos);

	//! Выделить семплы.
	bool SelectSamples(long startSample, long stopSample, bool autoScale);

	//! Плучить выделенные 
	QPair<long, long> GetSelectedSamples() const;

	//! Преобразует из координит мыши в коордитаты графика.
	QPoint FromMouseToPlot(const QPoint & pos) const;

	//! Получить веделенный прямугольние в координатах графика.
	QRect GetSelectedRect() const;

	//! Получить прямоугольник графика.
	const QRect & GetPlotRect() const ;

	//! Получить прямоугольник горизонтальной линейки.
	const QRect & GetHRullerRect() const;

	//! Получить прямоугольник вертикальной линейки.
	const QRect & GetVRullerRect() const;

	//! Получить прямоугольник контролов.
	const QRect & GetControlsRect() const;

	//! Получить прямоугольник перемещения скрола.
	const QRect & GetScrollRect() const;
	
	//! Установить текущую позицию проигрывания.
	bool SetCurrentPlayPos(long samples);

	//! Возвращает текущую позицию проигрывания.
	long GetCurrentPlayPos() const;

	//! Показать позицию.
	void SetVisibleCurrentPlayPos(bool visible);

	bool IsVisibleCurrentPlayPos() const;

	//! Переводит секунды в семплы, если forSelect == true то 
	inline long ToSelectedSample(qreal sample) const { return sample + 1.;}

	inline long ToViewSample(qreal sample) const { return sample + 0.5;}

	long ToSample(qreal sec) const;

	//! Перевести семплы в секунды.
	qreal ToSec(long sample) const;

	//! Отображать семплы/вермя
	void DisplaySamples(bool samples);

	//! Отображать семплы.
	bool IsDisplaySamples() const;

	ButtonsController & GetButtonController() const { return *markingController;}

	inline DataFrame * GetWaveData() const {return data;}

	inline void SetPhraseController(PhraseController * _c){controller = _c;}

	inline PhraseController * GetController() const {return controller;}

	bool ShowWave(bool wawe);

	bool IsWave() const;

	void SetMenuView(QMenu * _menu_view) {menu_view = _menu_view; }

	void SetMenuEdit(QMenu * _menu_edit) {menu_edit = _menu_edit; }

	void EnablePlayPosTracking(bool e) { playPosTracking = e; }

	bool IsPlayPosTracking() const {return playPosTracking;}

	void MoveLeft_LeftMarkerBorder();

	void MoveRight_LeftMarkerBorder();

	void MoveLeft_RightMarkerBorder();

	void MoveRight_RightMarkerBorder();

	void ZoomInVertical();

	void ZoomOutVertical();

	void ZoomInHorizontal();

	void ZoomOutHorizontal();

	void SetSelected(bool value){if (value != is_selected) {is_selected = value; update();}};

	bool IsSelected() const {return is_selected;};

	void setHRullerVisible(bool visible);

	bool isHRullerVisible() const {return is_h_ruller_visible;};

	void setScrollVisible(bool visible);

	bool isScrollVisible() const {return is_scroll_visible;}

	void setControllsHeightVisible(bool visible);

	bool isControllsHeightVisible() const {return is_controlls_height_visible;}



public slots:
	void ScaleHRuller(qreal hWindowCenter, int mouseDelta);

	void ScaleVRuller(qreal vWindowCenter, int mouseDelta);

signals:
	//! Сигнал выделены данные.
	void SelectWindow(const QRectF & rect);

	//! Изменения выделенных фреймов.
	void SelectedSamplesChanged(long startFrame, long endFrame);

	//! Позиция изменилась
	void PosChanged(long sample, double amplitude);

	//! Окно изменилось.
	void WindowChanged(const QRectF & window);

	//! бросили файл на окно.
	void DropFile(const QString & fileName);

	//! Сигнал изменился
	void DataChanged(long);

	//! Сигнал нажали  кнопку 
	void MouseRelease(const QMouseEvent * event);

	void MousePress(const QMouseEvent * event);

protected:

	//! Перерисовать.
	virtual void paintEvent(QPaintEvent * event);

	//! Перемещение мышки.
	virtual void mouseMoveEvent(QMouseEvent * event);

	//! Нажали на кнопку мышки.
	virtual void mousePressEvent(QMouseEvent * event);

	//! Отжали кнопку мышки.
	virtual void mouseReleaseEvent(QMouseEvent * event);
	
	//! Двойной клик.
	virtual void mouseDoubleClickEvent(QMouseEvent * event);

	//! Прокрутка.
	virtual void wheelEvent(QWheelEvent * event);

	//! Перемещение курсора
	virtual void keyPressEvent(QKeyEvent * event);

	//! Обновим размеры.
	virtual void resizeEvent(QResizeEvent * event);

	void dragEnterEvent(QDragEnterEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void dragLeaveEvent(QDragLeaveEvent *event);

	void dropEvent(QDropEvent *event);

private:
	//! Текущее событие.
	enum Action
	{
		//! Нет события.
		NoAction,

		//! Смещение по горизонтали.
		MoveHorizontal,

		//! Смещение по вертикали.
		MoveVertical,

		//! Выделение.
		SelectActon,

		//! Смещение контролов.
		MoveControls
	};

	QByteArray samples;

	//! Вертикальная линейка.
	VRullerFrame * vRullerFrame;

	//! Горизонтальная линейка.
	HRullerFrame * hRullerFrame;

	//! Граффик.
	DataFrame * data;

	//! Окно.
	QRectF window;

	//! положение курсора.
	QPoint curPos;

	//! Состояние мышки.
	Qt::MouseButtons mouseState;

	//! Начало выделения.
	QPoint startPos;

	//! Разрешить выделение.
	bool enableSelectWindow;

	//! Разрешить масштабирование.
	bool enableTanslate;

	//! разршить выделять треки.
	bool enableSelectSamples;

	//! Разрешить перемещать контролы.
	bool enableMoveControl;

	//! начало и конец выделения.
	long startSample, endSample, start_select_sample, end_select_sample;

	//! Полное окно.
	QRectF maxWindow;

	//! Минимальный размер окна.
	QSizeF minWindowSize;

	//! Отступы
	int leftMargin;
	int rightMargin;
	int topMargin;
	int bootomMargin;
	int hRullerHigth;
	int vRullerWidth;
	int scrollHeight;
	int controllsHeight;

	//! Текущее событие.
	Action curAction;

	//! Текущая позиция воспроизведения.
	long currentPlayPos;

	//! Показывать 
	bool visibleCurrentPlayPos;

	//! Разметка элементов виджета.
	QRect scrollRect;
	QRect plotRect;
	QRect vRullerRect;
	QRect hRullerRect;
	QRect controllRect;

	ScrollButton * scrollButton;

	SelectButton * startSelector;

	SelectButton * endSelector;

	//! Контроллер меток.
	ButtonsController * markingController;

	//! Контроллер селекторов.
	ButtonsController * selectorController;
	
	//! Контроллер скрола
	ButtonsController * scrollController;

	Button * curButton;

	QTimer * moveTimer;

	QPointF offset;

	PhraseController * controller;

	bool showWave;
	
	QMenu * menu_view;

	QMenu * menu_edit;

	bool playPosTracking;

	int key_repeat_count;

	bool is_selected;

	bool is_h_ruller_visible;

	bool is_scroll_visible;

	bool is_controlls_height_visible;

	//! Обновить схему разметки.
	void UpdateScheme();

	//! Рисуем окно выбора.
	void DrawSelectedWindow(QPainter & painter);

	//! Рисуем выделенные семплы.
	void DrawSelectedSamples(QPainter & painter);

	//! Сигнал выделение изменилось.
	void EmitSamplesChanged();

	//! Нарисовать позицию воспроизведения.
	void DrawPlayPos(QPainter & painter);

	//!  Нарисовать разметку.
	void DrawMarking(QPainter & painter);

	void DrawScroll(QPainter & painter);

	void DrawSelectors(QPainter & painter);

	Button * SelectAndPressButton(QMouseEvent * event);

	Button * GetButton(const QPoint & pos);

	void CheckSelected();

private slots:
	void OnMoveTimeout();

	//! Обновить окна.
	void UpdateWindows();

	void OnUpdate();

	void OnCustomContextMenuRequested(const QPoint &);
};

#endif // WAVEVIEWER1_H
