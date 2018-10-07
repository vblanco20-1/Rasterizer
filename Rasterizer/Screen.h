#pragma once

#include <SDL.h>
#include <SDL_render.h>
#include <vector>
#include "Color.h"

struct Sprite
{
	uint8_t posX;
	uint8_t posY;
	uint8_t Tile;
	uint8_t Flags;

};



class Screen
{
public:
	Screen();
	~Screen();

	
	void SetPixel(unsigned short x, unsigned short y, Color color,float Depth);
	

	void Clear();
	void DrawFrame();
	




	

private:
	
	SDL_Texture * tileMap;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

	std::vector< uint8_t > tileMapPixels;
	std::vector<float> depthBuffer;

};

