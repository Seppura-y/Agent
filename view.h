#pragma once
#include<mutex>
struct AVCodecParameters;
struct AVFrame;
class View
{
public:
	enum PixFormat {
		YUV420P = 0,
		ARGB = 25,
		RGBA = 26,
		BGRA = 28
	};

	enum RenderType {
		SDL = 0,
		OPENGL = 1
	};

	static View* CreateView(RenderType type = SDL);

	virtual void Close() = 0;
	virtual bool InitView(int width, int height, PixFormat format, void* winID = nullptr) = 0;
	bool InitView(AVCodecParameters* param);
	virtual bool DrawView(const unsigned char* data, int linesize) = 0;
	virtual bool DrawView(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch) = 0;
	bool DrawFrame(AVFrame* frame);
	void Scale(int width, int height);
	int getFps();
	void set_win(void* win) { win_id_ = win; };

protected:
	void* win_id_ = nullptr;
	int width_ = 0;
	int height_ = 0;
	int scale_width_ = 0;
	int scale_height_ = 0;
	int fps_record_ = 0;
	int draw_count_ = 0;
	int begin_ms_ = 0;
	PixFormat format_ = YUV420P;
	std::mutex mtx_;
};

