#include "XFormat.h"
#include "XTools.h"
#include<iostream>
extern"C"
{
#include<libavformat/avformat.h>
}
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

bool XFormat::isTimeout()
{
	//unique_lock<mutex> lock(mtx_);
	if (NowMs() - last_read_time_ > timeout_ms_)
	{
		is_connected_ = false;
		last_read_time_ = NowMs();
		return true;
	}
	return false;
}

static int TimeoutCallBack(void * p)
{
	XFormat* xfmt = (XFormat*)p;
	if (xfmt->isTimeout())
	{
		//cout << "TimeoutCallBack  :  timeout" << endl;
		return 1;
	}
	return 0;
}

void XFormat::set_timeout(int time_ms)
{
	unique_lock<mutex> lock(mtx_);
	timeout_ms_ = time_ms;
	if (fmCtx_)
	{
		AVIOInterruptCB cb = { TimeoutCallBack,this };
		fmCtx_->interrupt_callback = cb;
	}
}

void XFormat::set_ctx(AVFormatContext* ctx)
{
	unique_lock<mutex> fm_lock(mtx_);
	if (fmCtx_)
	{
		if (fmCtx_->oformat)
		{
			avio_closep(&fmCtx_->pb);
			avformat_free_context(fmCtx_);
			fmCtx_ = nullptr;
		}
		else if (fmCtx_->iformat)
		{
			avformat_close_input(&fmCtx_);
		}
		else
		{
			avformat_free_context(fmCtx_);
			fmCtx_ = nullptr;
		}
	}
	fmCtx_ = ctx;
	if (!fmCtx_)
	{
		is_connected_ = false;
		return;
	}
	is_connected_ = true;

	last_read_time_ = NowMs();

	if (timeout_ms_ > 0)
	{
		AVIOInterruptCB cb = { TimeoutCallBack,this };
		fmCtx_->interrupt_callback = cb;
	}

	audio_index_ = -1;
	video_index_ = -1;

	if (fmCtx_)
	{
		for (int i = 0; i < fmCtx_->nb_streams; i++)
		{
			if (fmCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audio_index_ = fmCtx_->streams[i]->index;
				audio_time_base_.num = fmCtx_->streams[i]->time_base.num;
				audio_time_base_.den = fmCtx_->streams[i]->time_base.den;
			}

			if (fmCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				video_index_ = fmCtx_->streams[i]->index;
				video_time_base_.num = fmCtx_->streams[i]->time_base.num;
				video_time_base_.den = fmCtx_->streams[i]->time_base.den;
				codec_id_ = fmCtx_->streams[i]->codecpar->codec_id;
				pix_format_ = fmCtx_->streams[i]->codecpar->format;
			}
		}
	}

}

bool XFormat::CloseContext()
{
	unique_lock<mutex> lock(mtx_);
	try {
		if (fmCtx_)
		{
			if (fmCtx_->oformat)
			{
				avio_closep(&fmCtx_->pb);
				avformat_free_context(fmCtx_);
				fmCtx_ = nullptr;
			}
			else if (fmCtx_->iformat)
			{
				avformat_close_input(&fmCtx_);
			}
			else
			{
				avformat_free_context(fmCtx_);
				fmCtx_ = nullptr;
			}
		}
	}
	catch (exception e)
	{
		return false;
	}

	return true;
}

int XFormat::CopyParameters(int index, AVCodecContext* ctx)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_)return -1;
	int ret = 0;
	if (index == video_index_)
	{
		ret = avcodec_parameters_to_context(ctx, fmCtx_->streams[video_index_]->codecpar);
		IERR(ret);
		return 0;
	}
	else if (index == audio_index_)
	{
		ret = avcodec_parameters_to_context(ctx, fmCtx_->streams[audio_index_]->codecpar);
		IERR(ret);
		return 0;
	}

	return -1;
}

int XFormat::CopyParameters(int index, AVCodecParameters* param)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_)return -1;
	int ret = 0;
	if (index == video_index_)
	{
		ret = avcodec_parameters_copy(param, fmCtx_->streams[video_index_]->codecpar);
		IERR(ret);
		return 0;
	}
	else if (index = audio_index_)
	{
		ret = avcodec_parameters_copy(param, fmCtx_->streams[audio_index_]->codecpar);
		IERR(ret);
		return 0;
	}
	return -1;

}

std::shared_ptr<XParam> XFormat::CopyVideoParameters()
{
	int index = get_video_index();
	shared_ptr<XParam> param;
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || fmCtx_ == nullptr)
	{
		return param;
	}

	param.reset(XParam::Create());
	*param->time_base = fmCtx_->streams[index]->time_base;
	avcodec_parameters_copy(param->para, fmCtx_->streams[index]->codecpar);

	return param;
}

std::shared_ptr<XParam> XFormat::CopyAudioParameters()
{
	int index = get_audio_index();
	shared_ptr<XParam> param;
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || fmCtx_ == nullptr)
	{
		return param;
	}

	param.reset(XParam::Create());
	*param->time_base = fmCtx_->streams[index]->time_base;
	avcodec_parameters_copy(param->para, fmCtx_->streams[index]->codecpar);

	return param;
}