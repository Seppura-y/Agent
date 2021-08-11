#pragma once
#include "XTools.h"
#include "XThread.h"
#include "XDecode.h"
class XDecodeThread : public XThread
{
public:
	bool Open(AVCodecParameters* param);
	void Do(AVPacket* packet) override;
	void set_next(XThread* thread) override;
	void do_next(AVPacket* packet) override;
	void Task()override;
	AVFrame* GetFrame();

private:
	std::mutex mtx_;
	bool is_need_play_ = false;
	XDecode decode_;
	XThread* next_ = nullptr;
	XAVPacketList pkt_list_;
	AVFrame* frame_ = nullptr;
};

