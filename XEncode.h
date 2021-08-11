#pragma once
#include "XCodec.h"
#include <vector>

#include <Windows.h>
static int GetCpuNumber()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (int)info.dwNumberOfProcessors;
}

struct AVPacket;
class XEncode :
    public XCodec
{
public:
    AVPacket* Encode(const AVFrame* frame);
    int Send(AVFrame* frame);
    int Recv(AVPacket* packet);

    std::vector<AVPacket*> GetEnd();
    std::vector<AVPacket*> End();
};

