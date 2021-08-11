#pragma once
#include <qwidget.h>
#include <QMenu>
#include <QString>
#include "capture_thread.h"
#include "XEncodeThread.h"
#include "XMuxThread.h"
#include "view.h"

class CaptureView : public QWidget
{
	Q_OBJECT

public:
	CaptureView(QWidget* p = nullptr);

	////////////////////////////////////////////////////////////////////////////////////////////
	//events
	void paintEvent(QPaintEvent* ev) override;
	void contextMenuEvent(QContextMenuEvent* ev)override;
	void timerEvent(QTimerEvent* ev)override;
	void resizeEvent(QResizeEvent* ev)override;


	///////////////////////////////////////////////////////////////////////////////////////////
	//user
	void Draw();
public slots:
	void StartPush();
	void StopPush();

private:
	//XEncodeThread encode_th_;
	int output_width_ = -1;
	int output_height_ = -1;
	QString rtmp_url_;
	View* view_ = nullptr;
	XMuxThread* mux_th_ = nullptr;
	XEncodeThread* encode_th_ = nullptr;
	CaptureThread* capture_th_ = nullptr;
	QMenu menu_;
};

