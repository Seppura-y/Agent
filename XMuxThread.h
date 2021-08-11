#pragma once
#include "XThread.h"
#include "XMux.h"
#include "XTools.h"
class XMuxThread : public XThread
{
public:
	//bool Open(std::string url,AVCodecParameters* vparam,AVRational* vtimebase,AVCodecParameters* aparam,AVRational* atimebase,int timeout,uint8_t* exData,int exSize);
	bool Open(std::string url, AVCodecParameters* vparam, AVRational* vtimebase, AVCodecParameters* aparam, AVRational* atimebase, int timeout);
	void Do(AVPacket* packet);
	void Task();
private:
	bool isRtmp_ = false;
	int time_out_ = 0;
	XMux xmux_;
	std::string url_;
	std::mutex mtx_;
	XAVPacketList pkt_list_;
	XParam* vParam_ = nullptr;
	XParam* aParam_ = nullptr;
};

