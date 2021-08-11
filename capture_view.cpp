#include "capture_view.h"
#include "view.h"

extern"C"
{
#include "libavcodec/avcodec.h"
}
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QDebug>
#include <QContextMenuEvent>
#include <QTimer>



#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;
CaptureView::CaptureView(QWidget* p) : QWidget(p)
{
	this->setStyleSheet("background-color: rgb(55,55,55)");
	//rtmp_url_ = QString("rtmp://107.172.153.24/live/livestream");
	rtmp_url_ = QString("rtmp://192.168.1.152/live/livestream");
	//rtmp_url_ = QString("./video/1234.flv");

	QAction* a = menu_.addAction(QString::fromLocal8Bit("开始推流"));
	QObject::connect(a, SIGNAL(triggered()), this, SLOT(StartPush()));
	a->setEnabled(true);

	a = menu_.addAction(QString::fromLocal8Bit("停止推流"));
	a->setEnabled(false);
	QObject::connect(a, SIGNAL(triggered()), this, SLOT(StopPush()));

	view_ = View::CreateView();
	view_->InitView(this->size().width(), this->size().height(), View::YUV420P, (void*)this->winId());

	bool ret = true;
	capture_th_ = new CaptureThread();
	ret = capture_th_->Init();
	if (!ret)
	{
		qDebug() << "capture_th_->Init() failed";
		return;
	}

	//output_width_ = capture_th_->getInputWidth();
	//output_height_ = capture_th_->getInputHeight();
	output_width_ = 800;
	output_height_ = 600;
	
	ret = capture_th_->InitScale(output_width_, output_height_);
	if (!ret)
	{
		qDebug() << "capture_th_->InitScale(inWidth, inHeight) failed";
		return;
	}



	encode_th_ = new XEncodeThread();
	ret = encode_th_->Open(output_width_, output_height_);
	if (!ret)
	{
		qDebug() << "encode_th_->Open(inWidth, inHeight) failed";
		return;
	}
	auto param = encode_th_->CopyVideoParam();

	//uint8_t* exData = nullptr;
	//int exSize = 0;
	//encode_th_->GetCodecExtraData(exData, exSize);
	mux_th_ = new XMuxThread();
	//ret = mux_th_->Open(rtmp_url_.toStdString(), param->para, param->time_base, nullptr, nullptr,1000,exData,exSize);
	//ret = mux_th_->Open(rtmp_url_.toStdString(), param->para, param->time_base, nullptr, nullptr, 1000);
	//if (!ret)
	//{
	//	qDebug() << "mux_th_->Open failed";
	//	return;
	//}

	encode_th_->set_next(mux_th_);
	capture_th_->set_next(encode_th_);

	//mux_th_->Start();
	//this_thread::sleep_for(50ms);

	encode_th_->Start();
	this_thread::sleep_for(50ms);

	capture_th_->Start();


	startTimer(1);
}

void CaptureView::paintEvent(QPaintEvent* ev)
{
	QStyleOption opt;
	opt.init(this);

	QPainter painter(this);

	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void CaptureView::contextMenuEvent(QContextMenuEvent* ev)
{
	menu_.exec(QCursor::pos());
	ev->accept();
	qDebug() << "context menu event";
}

void CaptureView::timerEvent(QTimerEvent* ev)
{
	this->Draw();
}

void CaptureView::resizeEvent(QResizeEvent* ev)
{
	if (view_)
	{
		view_->Close();
		view_->InitView(output_width_, output_height_, View::YUV420P, (void*)this->winId());
		qDebug() << "width : " << this->width() << "height : " << this->height();
		qDebug() << "out width : " << output_width_ << " out height : " << output_height_;
	}
}

void CaptureView::StartPush()
{
	if (!encode_th_ || !mux_th_)
	{
		return;
	}
	else
	{
		encode_th_->setNeedPush(true);
		auto param = encode_th_->CopyVideoParam();
		//uint8_t* exData = nullptr;
		//int exSize = 0;
		//encode_th_->GetCodecExtraData(exData, exSize);
		//int ret = mux_th_->Open(rtmp_url_.toStdString(), param->para, param->time_base, nullptr, nullptr, 1000,exData,exSize);
		int ret = mux_th_->Open(rtmp_url_.toStdString(), param->para, param->time_base, nullptr, nullptr, 1000);
		if (!ret)
		{
			qDebug() << "mux_th_->Open failed";
			return;
		}

		mux_th_->Start();
		menu_.actions().at(0)->setEnabled(false);
		menu_.actions().at(1)->setEnabled(true);
	}
}

void CaptureView::StopPush()
{
	if (!encode_th_ || !mux_th_)
	{
		return;
	}
	else
	{
		encode_th_->setNeedPush(false);
		mux_th_->Stop();
		menu_.actions().at(0)->setEnabled(true);
		menu_.actions().at(1)->setEnabled(false);
	}
}

void CaptureView::Draw()
{
	if (!capture_th_ || !view_)
	{
		return;
	}
	AVFrame* frame = capture_th_->GetFrame();
	if (!frame)
	{
		return;
	}
	view_->DrawFrame(frame);
	av_frame_unref(frame);
	av_frame_free(&frame);
}