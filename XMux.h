#pragma once
#include "XFormat.h"
struct AVRational;
struct AVPacket;
class XMux : public XFormat
{
public:
    ~XMux();
    //static AVFormatContext* OpenContext(const char* url, AVCodecParameters* vparam, AVRational* vtimebase, AVCodecParameters* aparam, AVRational* atimebase);
    static AVFormatContext* OpenContext(const char* url, AVCodecParameters* vparam = nullptr, AVCodecParameters* aparam = nullptr,bool isRtmp = false);

    int WriteHeader();
    int WriteData(AVPacket* pkt);
    int WriteTrailer();
    int TimeScale(int index, AVPacket* pkt,AVRational src, int pts);
    int TimeScale(AVPacket* packet, AVRational* timebase, long long offset);

    void set_src_video_timebase(AVRational* timebase);
    void set_src_audio_timebase(AVRational* timebase);
private:
    bool isRtmp_ = false;
    long long vbegin_pts_ = -1;
    long long abegin_pts_ = -1;
    AVRational* v_src_timebase_ = nullptr;
    AVRational* a_src_timebase_ = nullptr;
};

