#include "XMuxThread.h"
#include "XTools.h"
#include <mutex>
#include <iostream>
extern"C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

using namespace std;


//bool XMuxThread::Open(std::string url, AVCodecParameters* vparam, AVRational* vtimebase, AVCodecParameters* aparam, AVRational* atimebase,int timeout,uint8_t* exData,int exSize)
//{
//	if (!url.c_str())return false;
//	if (strstr(url.c_str(), "rtmp"))
//	{
//		isRtmp_ = true;
//	}
//	unique_lock<mutex> lock(mtx_);
//	url_ = url;
//	time_out_ = timeout;
//	if (!aParam_)
//	{
//		aParam_ = XParam::Create();
//	}
//	if (!vParam_)
//	{
//		vParam_ = XParam::Create();
//	}
//	aParam_->para = aparam;
//	vParam_->para = vparam;
//	AVFormatContext* fmCtx = xmux_.OpenContext(url_.c_str(), vParam_->para, aParam_->para,isRtmp_);
//	if (!fmCtx)return false;
//	int vsIndex = av_find_best_stream(fmCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
//	fmCtx->streams[vsIndex]->codecpar->extradata = exData;
//	fmCtx->streams[vsIndex]->codecpar->extradata_size = exSize;
//	xmux_.set_ctx(fmCtx);
//	av_dump_format(fmCtx, 0, url_.c_str(), 1);
//	if (!vtimebase && !atimebase)
//	{
//		return false;
//	}
//	else if (!vtimebase)
//	{
//		aParam_->time_base = atimebase;
//		xmux_.set_src_audio_timebase(aParam_->time_base);
//	}
//	else if (!atimebase)
//	{
//		vParam_->time_base = vtimebase;
//		xmux_.set_src_video_timebase(vParam_->time_base);
//	}
//	else
//	{
//		aParam_->time_base = atimebase;
//		vParam_->time_base = vtimebase;
//		xmux_.set_src_video_timebase(vParam_->time_base);
//		xmux_.set_src_audio_timebase(aParam_->time_base);
//	}
//	//xmux_.set_src_video_timebase(vtimebase);
//	//xmux_.set_src_audio_timebase(atimebase);
//
//	//if (isRtmp_)
//	//{
//	//	xmux_.set_timeout(time_out_);
//	//}
//	return true;
//}

bool XMuxThread::Open(std::string url, AVCodecParameters* vparam, AVRational* vtimebase, AVCodecParameters* aparam, AVRational* atimebase, int timeout)
{
	if (!url.c_str())return false;
	if (strstr(url.c_str(), "rtmp"))
	{
		isRtmp_ = true;
	}
	unique_lock<mutex> lock(mtx_);
	url_ = url;
	time_out_ = timeout;
	if (!aParam_)
	{
		aParam_ = XParam::Create();
	}
	if (!vParam_)
	{
		vParam_ = XParam::Create();
	}
	aParam_->para = aparam;
	vParam_->para = vparam;
	AVFormatContext* fmCtx = xmux_.OpenContext(url_.c_str(), vParam_->para, aParam_->para, isRtmp_);
	if (!fmCtx)return false;

	xmux_.set_ctx(fmCtx);
	av_dump_format(fmCtx, 0, url_.c_str(), 1);
	if (!vtimebase && !atimebase)
	{
		return false;
	}
	else if (!vtimebase)
	{
		aParam_->time_base = atimebase;
		xmux_.set_src_audio_timebase(aParam_->time_base);
	}
	else if (!atimebase)
	{
		vParam_->time_base = vtimebase;
		xmux_.set_src_video_timebase(vParam_->time_base);
	}
	else
	{
		aParam_->time_base = atimebase;
		vParam_->time_base = vtimebase;
		xmux_.set_src_video_timebase(vParam_->time_base);
		xmux_.set_src_audio_timebase(aParam_->time_base);
	}
	//xmux_.set_src_video_timebase(vtimebase);
	//xmux_.set_src_audio_timebase(atimebase);

	//if (isRtmp_)
	//{
	//	xmux_.set_timeout(time_out_);
	//}
	return true;
}
void XMuxThread::Do(AVPacket* packet)
{
	//unique_lock<mutex> lock(mtx_);
	pkt_list_.Push(packet);
	//do_next(packet);
}
void XMuxThread::Task()
{

	xmux_.WriteHeader();

	while (!isExit_)
	{
		unique_lock<mutex> lock(mtx_);
		AVPacket* packet = pkt_list_.Pop();
		if (!packet || !packet->data)
		{
			MSleep(1);
			continue;
		}
		cout << packet->size<<flush;
		xmux_.WriteData(packet);
		cout << "w" << flush;
		av_packet_free(&packet);

		//if (!xmux_.isConnected())
		//{
		//	Open(url_, vParam_->para, vParam_->time_base,aParam_->para,aParam_->time_base, isRtmp_);
		//}
		MSleep(1);
	}

	xmux_.WriteTrailer();
	xmux_.set_ctx(nullptr);
	pkt_list_.Clear();
}