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

	BuildTileArray();
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
		p = 50;
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

void Screen::AddTriangleToTiles(const Triangle &tri)
{
	const auto v0 = tri.Positions[0];
	const auto v1 = tri.Positions[1];
	const auto v2 = tri.Positions[2];

	// Compute triangle bounding box
	float minX = std::min(v0.x, std::min(v1.x, v2.x));
	float minY = std::min(v0.y, std::min(v1.y, v2.y));
	float maxX = std::max(v0.x, std::max(v1.x, v2.x));
	float maxY = std::max(v0.y, std::max(v1.y, v2.y));

	//divide bounding box into tiles
	minX /= TileSize;
	minY /= TileSize;
	maxX /= TileSize;
	maxY /= TileSize;

	//calculate tile bounds
	short minX_t = floor(minX);
	short minY_t = floor(minY);

	short maxX_t =  ceil(maxX);
	short maxY_t =  ceil(maxY);

	//clamp to borders
	minX_t = std::max(minX_t, (short)0);
	minY_t = std::max(minY_t, (short)0);

	maxX_t = std::min(maxX_t, (short)xTiles);
	maxY_t = std::min(maxY_t, (short)yTiles);

	//iterate tiles and add to it
	for (auto y = minY_t; y < maxY; y++)
	{
		for (auto x = minX_t; x < maxX; x++)
		{
			const auto tileIDX = (y * xTiles) + x;

			Tiles[tileIDX].trianglequeue.enqueue(tri);
		}
	}

}

void Screen::BuildTileArray()
{
	//calculate amount of tiles
	xTiles = ScreenWidth / TileSize;
	yTiles = ScreenHeight / TileSize;
	
	//allocate tile array
	Tiles = std::vector<FramebufferTile>(xTiles * yTiles);

	//set the pixel bounds of the tiles
	for (int y = 0; y < yTiles; y++)
	{
		for (int x = 0; x < xTiles; x++)
		{
			FramebufferTile & tile = Tiles[ (y *xTiles)+x];
			tile.minY = y * TileSize;
			tile.minX = x * TileSize;

			tile.maxX = (x + 1) * TileSize;
			tile.maxY = (y + 1) * TileSize;

			if (tile.maxX > ScreenWidth) tile.maxX = ScreenWidth;

			if (tile.maxY > ScreenHeight) tile.maxY = ScreenHeight;


		}
	}
	
}

void Screen::DrawTile(short tile_x, short tile_y)
{
	FramebufferTile & tile = Tiles[(tile_y *xTiles) + tile_x];

	DrawTile(&tile);
	
	
}

void Screen::DrawTile(FramebufferTile * Tile)
{
	Triangle buffer[TileQueueTraits::BLOCK_SIZE];

	while (true)
	{
		//unload BLOCK_SIZE at a time
		size_t count = Tile->trianglequeue.try_dequeue_bulk(buffer, TileQueueTraits::BLOCK_SIZE);

		//empty queue
		if (count == 0) return;


		for (int i = 0; i < count; i++)
		{
			drawTri_Tile(buffer[i], Tile);
		}
	}
}

