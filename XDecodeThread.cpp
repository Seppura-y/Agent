#include "XDecodeThread.h"
#include <iostream>
extern"C"
{
#include<libavcodec/avcodec.h>
}
using namespace std;

static void PrintErr(int ret)
{
	char buffer[1024] = { 0 };
	av_strerror(ret, buffer, sizeof(buffer) - 1);
	cerr << buffer << endl;
}
#define PERR(err) if(err != 0){PrintErr(err);return nullptr;}
#define IERR(err) if(err != 0){PrintErr(err);return -1;}
#define BERR(err) if(err != 0){PrintErr(err);return false;}


void XDecodeThread::Do(AVPacket* packet)
{
	//cout << "#" << flush;
	if (packet->stream_index != 0)return;
	pkt_list_.Push(packet);
}
void XDecodeThread::set_next(XThread* thread)
{
	unique_lock<mutex> lock(mtx_);
	next_ = thread;
}
void XDecodeThread::do_next(AVPacket* packet)
{
	unique_lock<mutex> lock(mtx_);
	if (next_)
	{
		next_->Do(packet);
	}
}
void XDecodeThread::Task()
{
	{
		unique_lock<mutex> lock(mtx_);
		if(!frame_)
		frame_ = av_frame_alloc();
	}

	while (!isExit_)
	{
		AVPacket* packet = pkt_list_.Pop();
		if (!packet)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		int ret = decode_.Send(packet);
		av_packet_free(&packet);
		if (ret < 0)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		{
			unique_lock<mutex> lock(mtx_);
			int ret = decode_.Recv(frame_);
			if (ret >= 0)
			{
				cout << "$" << flush;
				is_need_play_ = true;
			}
		}

	}

	{
		unique_lock<mutex> lock(mtx_);
		if (frame_)
		{
			av_frame_free(&frame_);
		}
	}
}

bool XDecodeThread::Open(AVCodecParameters* param)
{
	if (!param)return false;
	unique_lock<mutex> lock(mtx_);
	AVCodecContext* cdCtx = decode_.CreateCodec(param->codec_id);
	if (cdCtx == nullptr)
	{
		LOGERROR("decode_.CreateCodec falied")
			return false;
	}
	int ret = avcodec_parameters_to_context(cdCtx, param);
	IERR(ret)
	
	decode_.set_ctx(cdCtx);

	bool re = decode_.OpenContext();
	if (!re)
	{
		LOGERROR("decode_.OpenContext failed")
			return false;
	}

	return true;
}

AVFrame* XDecodeThread::GetFrame()
{
	unique_lock<mutex> lock(mtx_);
	if (!frame_ || !is_need_play_)return nullptr;
	AVFrame* frame = av_frame_alloc();
	int ret = av_frame_ref(frame, frame_);
	if (ret != 0)
	{
		av_frame_free(&frame);
		return nullptr;
	}

	return frame;
}