#include "Screen.h"

#include <iostream>


using namespace std;



Screen::Screen()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = window = SDL_CreateWindow
	(
		"SDL2",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		ScreenWidth, ScreenHeight,
		SDL_WINDOW_SHOWN
	);

	renderer = SDL_CreateRenderer
	(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	cout << "Renderer name: " << info.name << endl;
	cout << "Texture formats: " << endl;
	for (Uint32 i = 0; i < info.num_texture_formats; i++)
	{
		cout << SDL_GetPixelFormatName(info.texture_formats[i]) << endl;
	}

	const unsigned int texWidth = ScreenWidth;
	const unsigned int texHeight = ScreenHeight;
	
	tileMap = SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		texWidth, ScreenHeight
	);


	vector< unsigned char > tilepx(texWidth * ScreenHeight * 4, 0);
	
	tileMapPixels = tilepx;

	vector<float > depthpx(texWidth * ScreenHeight, 0.0);
	depthBuffer = depthpx;
}


Screen::~Screen()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}



void Screen::Clear()
{
	for (auto &p : tileMapPixels)
	{
		p = 0;
	}
	for (auto &p : depthBuffer)
	{
		p = 100000.0f;
	}
}

void Screen::DrawFrame()
{
	
	//for (auto&p : tileMapPixels)
	//{
	//	p = rand() % 255;
	//}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	SDL_UpdateTexture
	(
		tileMap,
		NULL,
		//&backgroundpixels[0],
		&tileMapPixels[0],
		ScreenWidth * 4
	);

	
	SDL_RenderCopy(renderer, tileMap, nullptr, nullptr);
	SDL_RenderPresent(renderer);

	
}

