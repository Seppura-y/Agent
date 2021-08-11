#pragma once
#include "XCodec.h"
#include <vector>
struct AVPacket;
struct AVFrame;
struct AVCodecParserContext;
class XDecode : public XCodec
{
public:
	XDecode();
	int Send(AVPacket* packet);
	int Recv(AVFrame* frame);
	bool Parse(const unsigned char* data,int &datasize);
	AVCodecParserContext* CreateParser();
	void set_parser(AVCodecParserContext* psr);
	std::vector<AVPacket*> get_vec();

	std::vector<AVFrame*> GetEnd();
	std::vector<AVPacket*> vec;
	AVPacket* packet = nullptr;
private:

	AVCodecParserContext* parser_ = nullptr;
};