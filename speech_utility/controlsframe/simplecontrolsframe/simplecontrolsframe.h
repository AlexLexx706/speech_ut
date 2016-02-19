#ifndef SIMPLE_CONTROLS_H
#define SIMPLE_CONTROLS_H

#include "../controlsframe.h"
#include "../../speech_utility_global.h"

class SPEECH_UTILITY_EXPORT SimpleControlsFrame : public ControlsFrame
{
	Q_OBJECT

public:
	SimpleControlsFrame(QWidget *parent);
	~SimpleControlsFrame();
	virtual long GetSelectedStart();
	virtual long GetSelectedEnd();
	virtual void OnMarkingChanged(const QString & markingName, 
							bool pahrasesIntersecton,
							const QColor & color,
							const QString & fileName,
							const QString & maringStr);

	virtual void OnSelectedChanged(long start, long end);
	virtual void OnPlayChanged(bool play);
	virtual void SetEnabledPlayback(bool enable);
	virtual bool IsPlaybackEnabled();
	virtual void SetConfig(const QString & jsonString);

private slots:
	void on_pushButton_playStop_toggled(bool checked);

private:
	void *_ui;
	
};

#endif // SIMPLE_CONTROLS_H
