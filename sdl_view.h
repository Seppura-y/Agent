#pragma once
#include "view.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
class SDLView : public View
{
public:
	bool InitView(int width, int height, PixFormat format, void* winID) override;
	bool DrawView(const unsigned char* data, int linesize) override;
	bool DrawView(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch) override;
	void Close() override;

private:
	SDL_Window* window_ = nullptr;
	SDL_Renderer* renderer_ = nullptr;
	SDL_Texture* texture_ = nullptr;
};

