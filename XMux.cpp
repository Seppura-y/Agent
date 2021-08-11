#include "XMux.h"
#include "XTools.h"
#include<iostream>
extern"C"
{
#include<libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

static void PrintErr(int ret)
{
	char buffer[1024] = { 0 };
	av_strerror(ret, buffer, sizeof(buffer) - 1);
	cerr << buffer << endl;
}
#define PERR(err) if(err != 0){PrintErr(err);return nullptr;}
#define IERR(err) if(err != 0){PrintErr(err);return -1;}

//static int TimeoutCallBack(void * ptr)
//{
//	XFormat* xfmt = (XFormat*)ptr;
//	if (xfmt->isTimeout())
//	{
//		return 1;
//	}
//	return 0;
//}

XMux::~XMux()
{
	if (v_src_timebase_)
	{
		delete v_src_timebase_;
		v_src_timebase_ = nullptr;
	}
	if(a_src_timebase_)
	{
		delete a_src_timebase_;
		a_src_timebase_ = nullptr;
	}

}

void XMux::set_src_video_timebase(AVRational* timebase)
{
	if (!timebase)return;
	unique_lock<mutex> lock(mtx_);
	if (!v_src_timebase_)
	{
		v_src_timebase_ = new AVRational();
	}
	*v_src_timebase_ = *timebase;
}
void XMux::set_src_audio_timebase(AVRational* timebase)
{
	if (!timebase)return;
	unique_lock<mutex> lock(mtx_);
	if (!a_src_timebase_)
	{
		a_src_timebase_ = new AVRational();
	}
	*a_src_timebase_ = *timebase;
}


AVFormatContext* XMux::OpenContext(const char* url, AVCodecParameters* vparam, AVCodecParameters* aparam,bool isRtmp)
{
	AVFormatContext* fmCtx = nullptr;
	int ret = -1;
	if (isRtmp)
	{
		ret = avformat_alloc_output_context2(&fmCtx, NULL, "flv", url);
	}
	else
	{
		ret = avformat_alloc_output_context2(&fmCtx, NULL, NULL, url);
	}

	if (vparam)
	{
		AVStream* vs = avformat_new_stream(fmCtx, NULL);
		vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		avcodec_parameters_copy(vs->codecpar, vparam);
	}

	if (aparam)
	{
		AVStream* as = avformat_new_stream(fmCtx, NULL);
		as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
		avcodec_parameters_copy(as->codecpar, aparam);
	}
	//if (isRtmp)
	//{
	//	AVDictionary* opt = nullptr;
	//	av_dict_set(&opt, "stimeout", "10000", 1);

	//	AVIOInterruptCB cb = {TimeoutCallBack,fmCtx};
	//	ret = avio_open2(&fmCtx->pb, url, AVIO_FLAG_WRITE, &cb, &opt);

	//	if (opt)
	//	{
	//		av_dict_free(&opt);
	//	}
	//}
	

	ret = avio_open(&fmCtx->pb, url, AVIO_FLAG_WRITE);
	PERR(ret);

	return fmCtx;
}

int XMux::WriteHeader()
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_) return -1;
	int ret = avformat_write_header(fmCtx_, NULL);
	last_read_time_ = NowMs();
	IERR(ret);
	
	return 0;
}

int XMux::WriteTrailer()
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_) return -1;
	int ret = av_write_trailer(fmCtx_);
	IERR(ret);

	return 0;
}

int XMux::TimeScale(int index, AVPacket* pkt, AVRational src, int pts)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_) return -1;
	if (index == get_video_index())
	{
		pkt->pts = av_rescale_q_rnd(pkt->pts - pts, src, fmCtx_->streams[get_video_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
		pkt->dts = av_rescale_q_rnd(pkt->dts - pts, src, fmCtx_->streams[get_video_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
		pkt->duration = av_rescale_q_rnd(pkt->duration, src, fmCtx_->streams[get_video_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	}
	else if (index == get_audio_index())
	{
		pkt->pts = av_rescale_q_rnd(pkt->pts - pts, src, fmCtx_->streams[get_audio_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
		pkt->dts = av_rescale_q_rnd(pkt->dts - pts, src, fmCtx_->streams[get_audio_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
		pkt->duration = av_rescale_q_rnd(pkt->duration, src, fmCtx_->streams[get_audio_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	}
	pkt->pos = -1;
	return 0;
}

int XMux::TimeScale(AVPacket* packet, AVRational* timebase, long long offset)
{
	if (!packet || !timebase)return -1;
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_)return -1;
	AVStream* stream = fmCtx_->streams[packet->stream_index];
	packet->pts = av_rescale_q_rnd(packet->pts - offset, *timebase, stream->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	packet->dts = av_rescale_q_rnd(packet->dts - offset, *timebase, stream->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	packet->duration = av_rescale_q(packet->duration, *timebase, stream->time_base);
	packet->pos = -1;
	return 0;
}

int XMux::WriteData(AVPacket* pkt)
{
	if (!pkt)return -1;
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_) return -1;
	if (pkt->pts == AV_NOPTS_VALUE)
	{
		pkt->pts = 0;
		pkt->dts = 0;
	}
	if (pkt->stream_index == get_video_index())
	{
		if (vbegin_pts_ < 0)
		{
			vbegin_pts_ = pkt->pts;
		}

		AVRational time_base;
		time_base.den = v_src_timebase_->den;
		time_base.num = v_src_timebase_->num;
		lock.unlock();
		TimeScale(get_video_index(), pkt, time_base, vbegin_pts_);
		//TimeScale(pkt, v_src_timebase_, vbegin_pts_);
		lock.lock();
	}
	if (pkt->stream_index == get_audio_index())
	{
		if (abegin_pts_ < 0)
		{
			abegin_pts_ = pkt->pts;
		}
		AVRational time_base;
		time_base.den = a_src_timebase_->den;
		time_base.num = a_src_timebase_->num;
		lock.unlock();
		TimeScale(get_audio_index(), pkt, time_base, abegin_pts_);
		//TimeScale(pkt, a_src_timebase_, abegin_pts_);
		lock.lock();
	}
	

	int ret = av_interleaved_write_frame(fmCtx_, pkt);
	last_read_time_ = NowMs();
	IERR(ret);

	return 0;
}