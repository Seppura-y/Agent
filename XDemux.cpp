#include "XDemux.h"
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
AVFormatContext* XDemux::OpenContext(const char* url)
{
	AVFormatContext* fmCtx = nullptr;
	AVDictionary* dict = nullptr;
	av_dict_set(&dict, "stimeout", "1000000",0);
	int ret = avformat_open_input(&fmCtx, url, NULL, &dict);
	PERR(ret);
	av_dict_free(&dict);

	ret = avformat_find_stream_info(fmCtx, NULL);
	PERR(ret);

	av_dump_format(fmCtx, 0, url, 0);
	return fmCtx;
}

bool XDemux::Close()
{
	return true;
}

int XDemux::Read(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_)return -1;
	int ret = av_read_frame(fmCtx_, pkt);
	IERR(ret);
	last_read_time_ = NowMs();
	return ret;
}

int XDemux::Seek(int stream_index, long long pts)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmCtx_)return -1;
	int ret = av_seek_frame(fmCtx_, stream_index, pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
	IERR(ret);
	return 0;
}