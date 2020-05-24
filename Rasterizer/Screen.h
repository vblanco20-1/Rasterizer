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
	static const size_t BLOCK_SIZE = 1024;		// Use bigger blocks
};


using TileQueue = moodycamel::ConcurrentQueue<Triangle*, TileQueueTraits>;

struct FramebufferTile {
	//coordinates
	uint16_t minX, maxX, minY, maxY;
	TileQueue trianglequeue;
	
	FramebufferTile() {
		depthBuffer = new float[TileSize * TileSize];
		buffer = new Color[TileSize * TileSize];
	}
	~FramebufferTile() {
		delete[] depthBuffer;
		delete[] buffer;
	}

	void SetPixel(uint16_t x, uint16_t y,const Color& color, float Depth) {

		x -= minX;
		y -= minY;

		const unsigned int offset = (TileSize * y) + x;
		const char pxC = 0;

		const unsigned int idx = (TileSize * y) + x;
		if (Depth <= depthBuffer[idx])
		{
			
			buffer[idx] = color;
			depthBuffer[idx] = Depth;
		}
	}

	__forceinline Color &GetPixelFromCoordinates(uint16_t x, uint16_t y)const {

		x -= minX;
		y -= minY;
		const unsigned int idx = (TileSize * y) + x;
		return buffer[idx];
	}

	

	void Clear() {
		for (int i = 0; i < TileSize*TileSize; i++)
		{
			buffer[i] = Color(0.1f,0.1f,0.1f);
		}
		for (int i = 0; i < TileSize*TileSize; i++)
		{
			depthBuffer[i] = 100000.0f;
		}
	}
private:
	Color *buffer;//[TileSize * TileSize];
	float *depthBuffer;// [TileSize * TileSize];
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
	


	void AddTriangleToTiles(Triangle &tri);

	void BuildTileArray();

	void DrawTile(short tile_x, short tile_y);
	void DrawTile(FramebufferTile * Tile); 
		void BlitTile(FramebufferTile * Tile);

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

