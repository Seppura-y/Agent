#include "XEncode.h"
#include <iostream>
extern"C"
{
#include<libavcodec/avcodec.h>

}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

int XEncode::Send(AVFrame* frame)
{
	unique_lock<mutex> encode_lock(mtx_);
	if (!cdCtx_)return false;
	int ret = 0;
	if (!frame)
	{
		ret = avcodec_send_frame(cdCtx_, NULL);
	}
	else
	{
		ret = avcodec_send_frame(cdCtx_, frame);
	}

	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		cerr << "avcodec_send_frame : (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)" <<"ret : "<< ret<< endl;
		PrintError(ret);
		return -1;
	}
	else if (ret >= 0)
	{
		return ret;
	}
	else if (ret < 0)
	{
		cerr << "avcodec_Send_frame return < 0" << endl;
		PrintError(ret);
		return -1;
	}
}

int XEncode::Recv(AVPacket* packet)
{
	unique_lock<mutex> encode_lock(mtx_);
	if (!cdCtx_)return false;
	int ret = avcodec_receive_packet(cdCtx_, packet);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		cerr << "avcodec_receive_packet failed : (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)" << "ret : " << ret << endl;
		PrintError(ret);
		return -1;
	}
	else if (ret >= 0)
	{
		return ret;
	}
	else if (ret < 0)
	{
		cerr << "avcodec_receive_packet : return < 0" << endl;
		PrintError(ret);
		return -1;
	}
}


AVPacket* XEncode::Encode(const AVFrame* frame)
{
	if (!frame)return nullptr;
	unique_lock<mutex>lock(mtx_);
	if (!cdCtx_)return nullptr;

	//av_frame_make_writable((AVFrame*)frame);
	//发送到编码线程
	auto re = avcodec_send_frame(cdCtx_, frame);
	if (re != 0)return nullptr;
	auto pkt = av_packet_alloc();
	//接收编码线程数据
	re = avcodec_receive_packet(cdCtx_, pkt);
	if (re == 0)
	{
		return pkt;
	}
	av_packet_free(&pkt);
	if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
	{
		//cerr << "avcodec_receive_packet failed : (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)" << "ret : " << re << endl;
		//PrintError(re);
		return nullptr;
	}
	if (re < 0)
	{
		PrintError(re);
	}
	return nullptr;
}


vector<AVPacket*> XEncode::GetEnd()
{
	vector<AVPacket*> vec;
	unique_lock<mutex> lock(mtx_);
	if (!cdCtx_)return vec;
	int ret = XEncode::Send(nullptr);
	cout << "Send ret : " << ret << endl;
	while (ret >= 0)
	{
		AVPacket* packet = av_packet_alloc();
		ret = XEncode::Recv(packet);
		if (ret != 0)
		{
			av_packet_free(&packet);
			break;
		}
		vec.push_back(packet);
	}
	cout << "vec.size : " << vec.size() << endl;
	return vec;
}



std::vector<AVPacket*> XEncode::End()
{
	std::vector<AVPacket*> res;
	unique_lock<mutex>lock(mtx_);
	if (!cdCtx_)return res;
	auto re = avcodec_send_frame(cdCtx_, NULL); //发送NULL 获取缓冲
	if (re != 0)return res;
	while (re >= 0)
	{
		auto pkt = av_packet_alloc();
		re = avcodec_receive_packet(cdCtx_, pkt);
		if (re != 0)
		{
			av_packet_free(&pkt);
			break;
		}
		res.push_back(pkt);
	}
	return res;
}

