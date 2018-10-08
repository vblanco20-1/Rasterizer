#pragma once

#include <SDL.h>
#include <SDL_render.h>
#include <vector>
#include "Color.h"
#include "Constants.h"
#include "concurrentqueue.h"
#include "RasterizerMath.h"
struct Sprite
{
	uint8_t posX;
	uint8_t posY;
	uint8_t Tile;
	uint8_t Flags;

};


struct TileQueueTraits : public moodycamel::ConcurrentQueueDefaultTraits
{
	static const size_t BLOCK_SIZE = 256;		// Use bigger blocks
};


using TileQueue = moodycamel::ConcurrentQueue<Triangle, TileQueueTraits>;

struct FramebufferTile {
	//coordinates
	uint16_t minX, maxX, minY, maxY;
	TileQueue trianglequeue;
};

class Screen
{
public:
	Screen();
	~Screen();

	
	__forceinline void SetPixel(unsigned short x, unsigned short y, Color color,float Depth)	
	{
		const unsigned int offset = (ScreenWidth * 4 * y) + x * 4;
		const char pxC = 0;

		const unsigned int idx = (ScreenWidth * y) + x;
		if (Depth <= depthBuffer[idx])
		{
			tileMapPixels[offset + 0] = color.b;        // b
			tileMapPixels[offset + 1] = color.g;          // g
			tileMapPixels[offset + 2] = color.r;        // r

			tileMapPixels[offset + 3] = color.a;    // a		

			depthBuffer[idx] = Depth;

		}
	}

	void Clear();
	void DrawFrame();
	


	void AddTriangleToTiles(const Triangle &tri);

	void BuildTileArray();

	void DrawTile(short tile_x, short tile_y);
	void DrawTile(FramebufferTile * Tile);

	short GetXTiles() { return xTiles; };
	short GetYTiles() { return yTiles; };
	std::vector <FramebufferTile> Tiles;
private:
	
	SDL_Texture * tileMap;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

	std::vector< uint8_t > tileMapPixels;
	std::vector<float> depthBuffer;

	//std::vector <FramebufferTile> Tiles;

	short yTiles;
	short xTiles;

};

