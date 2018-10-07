/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include <functional>
#include "Constants.h"
#include "Screen.h"
#include "RasterizerMath.h"
#include<ctime>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

int main(int argc, char* args[])
{
	Screen screen;

	srand(time(0));
	Triangle testTri;

	testTri.Positions[0].y = 10;
	testTri.Positions[0].x = 100;
	testTri.Positions[0].z = 0;

	testTri.Positions[2].y = 150;
	testTri.Positions[2].x = 10;
	testTri.Positions[2].z = 0;

	testTri.Positions[1].y = 100;
	testTri.Positions[1].x = 200;
	testTri.Positions[1].z = 0;

	testTri.RandomizeColors();
	
	float rot = 0;
	while (true)
	{
		screen.Clear();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

		}

		glm::mat4 rotmat = glm::rotate(rot, glm::vec3(0.f, 0.f, 1.f));

		
		Triangle newtri = testTri.GetMultipliedByMatrix(rotmat);
		drawTri(newtri,[&](ScreenCoord p, float w0, float w1, float w2)
		{
			glm::vec4 color = w0 * testTri.Colors[0]+ w1* testTri.Colors[1]+ w2* testTri.Colors[2];
			screen.SetPixel(p.x, p.y, Color(color.r,color.g,color.b));
		}
		);


		screen.DrawFrame();

		rot += 0.1f;
		
	}
	/*
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);

			//Fill the surface white
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

			//Update the surface
			SDL_UpdateWindowSurface(window);

			//Wait two seconds
			SDL_Delay(2000);
		}
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	*/
	return 0;
}