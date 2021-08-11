#include "XCodec.h"
#include <iostream>
extern "C"
{
	#include<libavcodec/avcodec.h>
	#include<libavutil/opt.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

void PrintError(int errcode)
{
	char buffer[1024] = { 0 };
	av_strerror(errcode, buffer, sizeof(buffer) - 1);
	cerr << buffer << endl;
}

AVCodecContext* XCodec::get_codec_context()
{
	unique_lock<mutex> lock(mtx_);
	return cdCtx_;
}

AVCodecContext* XCodec::CreateCodec(int codecID, bool isDecode)
{
	AVCodec* codec;
	if (isDecode)
	{
		codec = avcodec_find_decoder((AVCodecID)codecID);
	}
	else
	{
		codec = avcodec_find_encoder((AVCodecID)codecID);
	}

	if (!codec)
	{
		cerr << "CreateCodec failed" << endl;
		return nullptr;
	}

	AVCodecContext* cdCtx = avcodec_alloc_context3(codec);
	if (!cdCtx)
	{
		cerr << "avcodec_alloc_context3 failed" << endl;
		return nullptr;
	}

	return cdCtx;
}

bool XCodec::SetOptions(const char* key, const char* value)
{
	unique_lock<mutex> codec_lock(mtx_);
	if (!cdCtx_)
	{
		cerr << "cdCtx is null" << endl;
		return false;
	}
	int ret = av_opt_set(cdCtx_->priv_data, key, value, 0);
	if (ret != 0)
	{
		PrintError(ret);
		return false;
	}

	return true;
}
bool XCodec::SetOptions(const char* key, const int value)
{
	unique_lock<mutex> codec_lock(mtx_);
	if (!cdCtx_)
	{
		cerr << "cdCtx is null" << endl;
		return false;
	}
	int ret = av_opt_set_int(cdCtx_->priv_data, key, value, 0);
	if (ret != 0)
	{
		PrintError(ret);
		return false;
	}
	return true;
}

void XCodec::set_ctx(AVCodecContext* ctx)
{
	unique_lock<mutex> codec_lock(mtx_);
	if (cdCtx_)
	{
		avcodec_free_context(&cdCtx_);
	}
	cdCtx_ = ctx;
}

bool XCodec::OpenContext()
{
	unique_lock<mutex> codec_lock(mtx_);
	if (!cdCtx_) 
	{
		cerr << "cdCtx_ is null" << endl;
		return false;
	}
	int ret = avcodec_open2(cdCtx_, NULL, NULL);
	if (ret != 0)
	{
		PrintError(ret);
		return false;
	}
	return true;
}

AVFrame* XCodec::CreateFrame()
{
	unique_lock<mutex> codec_lock(mtx_);
	AVFrame* frame = av_frame_alloc();
	if (!frame)
	{
		cerr << "av_frame_alloc failed" << endl;
		return nullptr;
	}

	frame->width = cdCtx_->width;
	frame->height = cdCtx_->height;
	frame->format = cdCtx_->pix_fmt;

	int ret = av_frame_get_buffer(frame, 0);
	if (ret != 0)
	{
		cerr << "av_farme_Get_buffer failed " << endl;
		av_frame_free(&frame);
		return nullptr;
	}

	return frame;
}

std::shared_ptr<XParam> XCodec::GetCodecParam()
{
	shared_ptr<XParam> param;
	unique_lock<mutex> lock(mtx_);
	if (!cdCtx_)
	{
		return param;
	}
	param.reset(XParam::Create());
	*param->time_base = cdCtx_->time_base;
	avcodec_parameters_from_context(param->para, cdCtx_);
	return param;
}

bool XCodec::GetCodecExtraData(uint8_t* exData,int exSize)
{
	unique_lock<mutex> lock(mtx_);
	if (!cdCtx_)
	{
		return false;
	}
	exData = cdCtx_->extradata;
	exSize = cdCtx_->extradata_size;
}