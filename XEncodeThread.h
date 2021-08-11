#pragma once
#include "XThread.h"
#include "XEncode.h"
#include "XTools.h"
class XEncodeThread : public XThread
{
public:

	bool Open(int oWidth, int oHeight);
	void Do(AVFrame* frame) override;

	void Task()override;

	void setNeedPush(bool bn);

	std::shared_ptr<XParam> CopyVideoParam();
	bool GetCodecExtraData(uint8_t* exData, int exSize);
private:
	std::mutex mtx_;
	bool is_need_push_ = false;
	XEncode vencode_;
	XThread* next_ = nullptr;
	XAVFrameList frame_list_;
	AVFrame* frame_ = nullptr;
	int encode_count_ = 0;
};

