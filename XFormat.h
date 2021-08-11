#pragma once
#include<mutex>
#include "XThread.h"
#include "XTools.h"
struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct XRational
{
	int num;
	int den;
};
class XFormat
{
public:
	int CopyParameters(int index,AVCodecContext* ctx);
	int CopyParameters(int index, AVCodecParameters* param);
	std::shared_ptr<XParam> CopyVideoParameters();
	std::shared_ptr<XParam> CopyAudioParameters();

	void set_ctx(AVFormatContext* ctx);
	int get_codec_id() { return codec_id_; };
	int get_pix_fmt() { return pix_format_; };
	int get_video_index(){ return video_index_; };
	int get_audio_index(){ return audio_index_; };
	XRational get_video_timebase() { return video_time_base_; };
	XRational get_audio_timebase() { return audio_time_base_; };

	void set_timeout(int time_ms);
	bool isTimeout();
	bool isConnected() { std::unique_lock<std::mutex> lock(mtx_); return is_connected_; };

	bool CloseContext();

protected:
	bool is_connected_ = false;
	int timeout_ms_ = 0;
	int last_read_time_ = 0;
	std::mutex mtx_;
	AVFormatContext* fmCtx_ = nullptr;
	int video_index_ = -1;
	int audio_index_ = -1;
	int codec_id_ = 0;
	int pix_format_ = 0;
	XRational video_time_base_ = { 1,25 };
	XRational audio_time_base_ = { 1,44100 };

};