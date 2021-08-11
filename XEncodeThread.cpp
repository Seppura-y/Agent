#include "XEncodeThread.h"
extern"C"
{
	#include"libavcodec/avcodec.h"
}

#pragma comment(lib,"avcodec.lib")

using namespace std;
bool XEncodeThread::Open(int oWidth,int oHeight)
{
	unique_lock<mutex> lock(mtx_);
	AVCodecContext* cdCtx = vencode_.CreateCodec(AV_CODEC_ID_H264, false);
	if (!cdCtx)
	{
		LOGERROR("encode_.CreateCodec() failed");
		return false;
	}
	
	cdCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	cdCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	cdCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	cdCtx->width = oWidth;
	cdCtx->height = oHeight;
	cdCtx->time_base = { 1,25 };
	cdCtx->framerate = { 25,1 };
	cdCtx->thread_count = GetCpuNumber();
	cdCtx->max_b_frames = 0;
	cdCtx->gop_size = 50;
	cdCtx->bit_rate = 300000; 
	//cdCtx->rc_max_rate = 30000;
	//cdCtx->rc_min_rate = 30000;
	//cdCtx->rc_buffer_size = 30000;
	

	vencode_.set_ctx(cdCtx);


	bool isok = vencode_.SetOptions("preset", "slow");
	if (!isok)
	{
		return false;
	}
	isok = vencode_.SetOptions("qp", 23);
	if (!isok)
	{
		return false;
	}
	isok = vencode_.SetOptions("tune", "zerolatency");
	if (!isok)
	{
		return false;
	}

	isok = vencode_.SetOptions("nal-hrd", "cbr");
	if (!isok)
	{
		return false;
	}

	bool ret = vencode_.OpenContext();
	if (!ret)
	{
		LOGERROR("encode_.OpenContext failed");
		return false;
	}

	return true;
}

void XEncodeThread::Do(AVFrame* frame)
{
	if (!frame)return;
	//unique_lock<mutex> lock(mtx_);
	if (!is_need_push_)
	{
		av_frame_unref(frame);
		//av_frame_free(&frame);
	}
	else
	{
		frame_list_.Push(frame);
	}

}


//void XEncodeThread::do_next(AVPacket* packet)
//{
//
//}

void XEncodeThread::Task()
{
	AVPacket* pkt = av_packet_alloc();
	while (!isExit_)
	{
		if (!is_need_push_)
		{
			this_thread::sleep_for(1ms);
			continue;
		}

		AVFrame* frame = frame_list_.Pop();
		if (!frame)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		frame->pts = (encode_count_++) * (vencode_.cdCtx_->time_base.den) / (vencode_.cdCtx_->time_base.num);

		int ret = vencode_.Send(frame);
		av_frame_free(&frame);
		//av_frame_unref(frame);
		if (ret != 0)
		{
			LOGERROR("encode_Send(frame) failed")
			this_thread::sleep_for(1ms);
			continue;
		}

		ret = vencode_.Recv(pkt);
		if (ret != 0)
		{
			LOGERROR("encode_.Recv(pkt) failed")
			this_thread::sleep_for(1ms);
			continue;
		}

		do_next(pkt);
		av_packet_unref(pkt);
		this_thread::sleep_for(1ms);
	}
}

void XEncodeThread::setNeedPush(bool bn)
{
	unique_lock<mutex> lock(mtx_);
	is_need_push_ = bn;
}

std::shared_ptr<XParam> XEncodeThread::CopyVideoParam()
{
	return vencode_.GetCodecParam();
}

bool XEncodeThread::GetCodecExtraData(uint8_t* exData, int exSize)
{

	vencode_.GetCodecExtraData(exData, exSize);
	return true;

}