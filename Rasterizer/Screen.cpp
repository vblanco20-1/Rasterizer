#include "Screen.h"

#include <iostream>
#include "Constants.h"

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

void Screen::SetPixel(unsigned short x, unsigned short y, Color color,float Depth)
{
	

	const unsigned int offset = (ScreenWidth * 4 * y) + x * 4;
	char pxC = 0;
	
	const unsigned int idx = (ScreenWidth * y) + x;
	if(Depth < depthBuffer[idx])
	{
		tileMapPixels[offset + 0] = color.b;        // b
		tileMapPixels[offset + 1] = color.g;          // g
		tileMapPixels[offset + 2] = color.r;        // r

		tileMapPixels[offset + 3] = color.a;    // a		

		depthBuffer[idx] = Depth;

	}

	
	

}

void Screen::Clear()
{
	for (auto &p : tileMapPixels)
	{
		p = 0;
	}
	for (auto &p : depthBuffer)
	{
		p = 1.0f;
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

