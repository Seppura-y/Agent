#include "XDemuxThread.h"
#include "XDemux.h"
#include<thread>
#include<iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;

void XDemuxThread::Stop()
{
	XThread::Stop();
	demux_.CloseContext();
}

void XDemuxThread::Do(AVPacket* packet)
{

}

void XDemuxThread::Task()
{
	AVPacket packet;
	LOGINFO("TASK START")
	while (!isExit_)
	{
		if (demux_.Read(&packet) >= 0)
		{
			//cout << "+" << flush;
			do_next(&packet);
			av_packet_unref(&packet);
			this_thread::sleep_for(1ms);
			//continue;
		}
		if (!demux_.isConnected())
		{
			Open(url_, timeout_);
		}
		this_thread::sleep_for(1ms);
	}
}

bool XDemuxThread::Open(std::string filename, int timeout)
{
	url_ = filename;
	timeout_ = timeout;
	demux_.set_ctx(nullptr);
	AVFormatContext* fmCtx = demux_.OpenContext(filename.c_str());
	if (!fmCtx)
	{
		LOGERROR("XDemuxThread::Open : open url failed")
		return false;
	}
	demux_.set_ctx(fmCtx);
	demux_.set_timeout(timeout);
	return true;
}

shared_ptr<XParam> XDemuxThread::CopyVideoParam()
{
	return demux_.CopyVideoParameters();
}
shared_ptr<XParam> XDemuxThread::CopyAudioParam()
{
	return demux_.CopyAudioParameters();
}