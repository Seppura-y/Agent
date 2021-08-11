#include "XDecode.h"
#include <iostream>
extern"C"
{
#include<libavcodec/avcodec.h>

}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

//static AVPacket* packet;

XDecode::XDecode()
{
	packet = av_packet_alloc();
}

int XDecode::Send(AVPacket* packet)
{
	unique_lock<mutex> decode_lock(mtx_);
	if (!cdCtx_)return -1;
	int ret = avcodec_send_packet(cdCtx_, packet);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		cerr << "avcodec_send_packet : (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)" << "ret : " << ret << endl;
		PrintError(ret);
		return -1;
	}
	else if (ret >= 0)
	{
		return ret;
	}
	else if (ret < 0)
	{
		cerr << "avcodec_Send_packet return < 0" << endl;
		PrintError(ret);
		return -1;
	}
	//if (ret != 0)
	//{
	//	return -1;
	//}
	//else
	//{
	//	return 0;
	//}
}


int XDecode::Recv(AVFrame* frame)
{
	unique_lock<mutex> decode_lock(mtx_);
	if (!cdCtx_)return -1;
	int ret = avcodec_receive_frame(cdCtx_, frame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		//cerr << "avcodec_receive_frame failed : (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)" << "ret : " << ret << endl;
		//PrintError(ret);
		return -1;
	}
	else if (ret >= 0)
	{
		return ret;
	}
	else if (ret < 0)
	{
		cerr << "avcodec_receive_frame : return < 0" << endl;
		PrintError(ret);
		return -1;
	}
}

vector<AVFrame*> XDecode::GetEnd()
{
	vector<AVFrame*> vec;
	int ret = Send(nullptr);
	if (ret < 0)return vec;
	while (ret >= 0)
	{
		AVFrame* frame = av_frame_alloc();
		ret = Recv(frame);
		if (ret < 0)
		{
			av_frame_free(&frame);
			break;
		}
		vec.push_back(frame);
	}
	cout << "vec.size : " << vec.size() << endl;
	return vec;
}

AVCodecParserContext* XDecode::CreateParser()
{
	//packet = av_packet_alloc();
	unique_lock<mutex> decode_lock(mtx_);
	if (cdCtx_ == nullptr) return nullptr;
	AVCodecParserContext* parser = av_parser_init(cdCtx_->codec_id);
	if (parser == nullptr)
	{
		cerr << "av_parser_init failed" << endl;
		return nullptr;
	}
	return parser;
}

bool XDecode::Parse(const unsigned char* data,int& datasize)
{
	unique_lock<mutex> decode_lock(mtx_);
	if (!cdCtx_ || !parser_ || !data)
	{
		return false;
	}
	const unsigned char* remaindata = data;
	while (datasize > 0)
	{
		AVPacket* packet = av_packet_alloc();
		av_packet_unref(packet);
		int parse_size = av_parser_parse2(parser_, cdCtx_,
			&packet->data, &packet->size,
			(unsigned char*)remaindata, datasize,
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

		datasize -= parse_size;
		remaindata += parse_size;

		if (packet->size)
		{
			vec.push_back(packet);
		}
		//else
		//{
		//	av_packet_free(&packet);
		//}
	}
	return true;
}
	


void XDecode::set_parser(AVCodecParserContext* psr)
{
	unique_lock<mutex> decode_lock(mtx_);
	if (parser_)
	{
		av_parser_close(parser_);
		parser_ = nullptr;
	}
	this->parser_ = psr;
}

vector<AVPacket*> XDecode::get_vec()
{
	unique_lock<mutex> decode_lock(mtx_);
	return vec;
}
