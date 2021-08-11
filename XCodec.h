#pragma once
#include <mutex>

#include "XTools.h"

void PrintError(int errcode);

struct AVCodecContext;
struct AVFrame;
class XCodec
{
public :
	static AVCodecContext* CreateCodec(int codecID, bool isDecode = true);
	void set_ctx(AVCodecContext* ctx);

	bool SetOptions(const char* key, const char* value);
	bool SetOptions(const char* key, const int value);

	bool OpenContext();

	AVFrame* CreateFrame();
	AVCodecContext* get_codec_context();
	std::shared_ptr<XParam> GetCodecParam();
	bool GetCodecExtraData(uint8_t* exData,int exSize);
public:
	AVCodecContext* cdCtx_ = nullptr;
protected:
	std::mutex mtx_;

};

