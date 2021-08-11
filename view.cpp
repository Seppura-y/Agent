#include "view.h"
#include "sdl_view.h"
#include "XTools.h"
//#include "XThread.h"
extern"C"
{
#include <libavcodec/avcodec.h>
}

#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avutil.lib")

using namespace std;

//long long NowMs()
//{
//	return (clock() / (CLOCKS_PER_SEC / 1000));
//}

bool View::InitView(AVCodecParameters* param)
{
	AVCodecID cID = param->codec_id;
	int format = 0;
	switch (param->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
		format = View::YUV420P;
		break;
	}

	return InitView(param->width, param->height, (View::PixFormat)format, win_id_);

}


View* View::CreateView(RenderType type)
{
	switch (type)
	{
	case SDL:
		return new SDLView();
	case OPENGL:
	default:
		break;
	}
	return nullptr;
}

void View::Scale(int width, int height)
{
	unique_lock<mutex> xv_lock(mtx_);
	scale_width_ = width;
	scale_height_ = height;
}

bool View::DrawFrame(AVFrame* frame)
{
	if (!frame->data[0] || !frame)
		return false;
	mtx_.lock();
	draw_count_++;
	if (begin_ms_ <= 0)
	{
		begin_ms_ = NowMs();
	}
	else
	{
		if (NowMs() - begin_ms_ >= 1000)
		{
			begin_ms_ = NowMs();
			fps_record_ = draw_count_;
			draw_count_ = 0;
		}
	}

	switch (frame->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
		mtx_.unlock();
		return DrawView(frame->data[0], frame->linesize[0],
			frame->data[1], frame->linesize[1],
			frame->data[2], frame->linesize[2]);
	case AV_PIX_FMT_ARGB:
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
		mtx_.unlock();
		return DrawView(frame->data[0], frame->linesize[0]);
	default:
		mtx_.unlock();
		return false;
	}
	return true;
}

int View::getFps()
{
	unique_lock<mutex> xv_lock(mtx_);
	return fps_record_;
}
