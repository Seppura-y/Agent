#pragma once
#include "XTools.h"
#include "XThread.h"
#include "XDemux.h"
//#include <iostream>
class XDemuxThread : public XThread
{
public:
	void Task()override;
	void Do(AVPacket* packet) override;
	void Stop() override;
	bool Open(std::string filename, int timeout = 1000);
	std::shared_ptr<XParam> CopyVideoParam();
	std::shared_ptr<XParam> CopyAudioParam();

private:
	std::string url_;
	int timeout_ = 0;
	XDemux demux_;
};

