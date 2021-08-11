#include "sdl_view.h"
#include <iostream>
#include <sdl/SDL.h>
extern "C"
{
#include<libavcodec/avcodec.h>
}

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

static bool InitLib()
{
	static mutex mt;
	static bool isFirstInit = true;
	unique_lock<mutex> init_lock(mt);
	if (isFirstInit)
	{
		int ret = SDL_Init(SDL_INIT_VIDEO);
		if (ret != 0)
		{
			cerr << SDL_GetError() << endl;
			return false;
		}
		isFirstInit = false;

		ret = SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		return true;
	}
	return true;
}

bool SDLView::InitView(int width, int height, PixFormat format, void* winID)
{
	if (!InitLib())
	{
		return false;
	}

	int fmt = 0;
	unique_lock<mutex> sdl_lock(mtx_);

	width_ = width;
	height_ = height;
	format_ = format;
	win_id_ = winID;

	if (win_id_ == nullptr)
	{
		window_ = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width_, height_, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	}
	else
	{
		window_ = SDL_CreateWindowFrom(win_id_);
	}
	if (window_ == nullptr)
	{
		cerr << "create window failed : " << SDL_GetError() << endl;
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
	if (renderer_ == nullptr)
	{
		cerr << "create renderer failed : " << SDL_GetError() << endl;
		return false;
	}

	switch (format_)
	{
	case View::YUV420P:
		fmt = SDL_PIXELFORMAT_IYUV;
		break;
	case View::ARGB:
		fmt = SDL_PIXELFORMAT_ARGB32;
		break;
	case View::RGBA:
		fmt = SDL_PIXELFORMAT_RGBA32;
		break;
	case View::BGRA:
		fmt = SDL_PIXELFORMAT_BGRA32;
		break;
	}

	texture_ = SDL_CreateTexture(renderer_, fmt, SDL_TEXTUREACCESS_STREAMING, width_, height_);
	if (texture_ == nullptr)
	{
		cerr << "create texture failed : " << SDL_GetError() << endl;
		return false;
	}

	return true;
}


bool SDLView::DrawView(const unsigned char* data, int linesize)
{
	unique_lock<mutex> sdl_lock(mtx_);
	if (!window_ || !renderer_ || !texture_ || !data)
	{
		cerr << "DrawView failed : (!window_ || !renderer_ || !texture_ || !data)" << endl;
		return false;
	}

	if (width_ <= 0 || height_ <= 0)
	{
		cerr << "DrawView failed : (width_ <= 0 || height_ <= 0)" << endl;
		return false;
	}

	if (linesize <= 0)
	{
		switch (format_)
		{
		case View::YUV420P:
		{
			linesize = width_;
			break;
		}
		case View::RGBA:
		case View::ARGB:
		case View::BGRA:
		{
			linesize = width_ * 4;
			break;
		}
		}
	}

	SDL_RenderClear(renderer_);

	int ret = SDL_UpdateTexture(texture_, NULL, data, linesize);
	if (ret != 0)
	{
		cerr << "SDL_UpdateTexture failed : " << SDL_GetError() << endl;
		return false;
	}

	SDL_Rect rect;
	SDL_Rect* pRect = nullptr;
	if (scale_width_ > 0 && scale_height_ > 0)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = scale_width_;
		rect.h = scale_height_;
		pRect = &rect;
	}
	ret = SDL_RenderCopy(renderer_, texture_, NULL, pRect);
	if (ret != 0)
	{
		cerr << "SDL_RenderCopy failed : " << SDL_GetError() << endl;
		return false;
	}

	SDL_RenderPresent(renderer_);
	return true;
}


bool SDLView::DrawView(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch)
{
	unique_lock<mutex> sdl_lock(mtx_);

	if (!window_ || !renderer_ || !texture_)
	{
		cerr << "DrawView failed : (!window_ || !renderer_ || !texture_)" << endl;
		return false;
	}

	if (!y || !u || !v || y_pitch <= 0 || u_pitch <= 0 || v_pitch <= 0)
	{
		cerr << "DrawView failed : (!y || !u || !v || y_pitch <=0 || u_pitch <= 0 || v_pitch <= 0)" << endl;
		return false;
	}

	if (width_ <= 0 || height_ <= 0)
	{
		cerr << "DrawView failed : (width_ <= 0 || height_ <= 0)" << endl;
		return false;
	}

	SDL_RenderClear(renderer_);

	int ret = SDL_UpdateYUVTexture(texture_, NULL, y, y_pitch, u, u_pitch, v, v_pitch);
	if (ret != 0)
	{
		cerr << "SDL_UpdateTexture failed : " << SDL_GetError() << endl;
		return false;
	}

	SDL_Rect rect;
	SDL_Rect* pRect = nullptr;
	if (scale_width_ > 0 && scale_height_ > 0)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = scale_width_;
		rect.h = scale_height_;
		pRect = &rect;
	}
	ret = SDL_RenderCopy(renderer_, texture_, NULL, pRect);
	if (ret != 0)
	{
		cerr << "SDL_RenderCopy failed : " << SDL_GetError() << endl;
		return false;
	}

	SDL_RenderPresent(renderer_);

	return true;
}

void SDLView::Close()
{
	unique_lock<mutex> lock(mtx_);
	if (texture_)
	{
		SDL_DestroyTexture(texture_);
	}

	if (renderer_)
	{
		SDL_DestroyRenderer(renderer_);
	}

	if (window_)
	{
		SDL_DestroyWindow(window_);
	}
}
