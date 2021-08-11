#pragma once
#include "XFormat.h"
struct AVPacket;
class XDemux : public XFormat
{
public:
   static AVFormatContext* OpenContext(const char* url);

   int Read(AVPacket* pkt);

   int Seek(int stream_index,long long pts);

   bool Close();
};

