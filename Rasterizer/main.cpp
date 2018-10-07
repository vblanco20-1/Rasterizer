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
#define OBJL_CONSOLE_OUTPUT
#include "OBJ_Loader.h"
#include <glm/gtx/transform.hpp>
#include <glm/geometric.hpp>

int main(int argc, char* args[])
{
	Screen screen;

	srand(time(0));
	Triangle testTri;

	objl::Loader MeshLoader;
	MeshLoader.LoadFile("dragon.obj");

	testTri.Positions[0].y = 1;
	testTri.Positions[0].x = 1;
	testTri.Positions[0].z = 0;

	testTri.Positions[2].y = 1.5;
	testTri.Positions[2].x = 1;
	testTri.Positions[2].z = 0;

	testTri.Positions[1].y = 1;
	testTri.Positions[1].x = 2;
	testTri.Positions[1].z = 0;

	testTri.RandomizeColors();
	
	float rot = 0;


	std::vector<Triangle> triangles;
	auto & vertices = (MeshLoader.LoadedMeshes[0].Vertices);
	auto & indices =  (MeshLoader.LoadedMeshes[0].Indices);
	for (int i = 0; i < indices.size(); i += 3)
	{
		objl::Vertex v0 = vertices[indices[i]];
		objl::Vertex v1 = vertices[indices[i+2]];
		objl::Vertex v2 = vertices[indices[i+1]];

		Triangle newTri;

		newTri.Positions[0].x = v0.Position.X;
		newTri.Positions[0].y = v0.Position.Y;
		newTri.Positions[0].z = v0.Position.Z;

		newTri.Positions[1].x = v1.Position.X;
		newTri.Positions[1].y = v1.Position.Y;
		newTri.Positions[1].z = v1.Position.Z;

		newTri.Positions[2].x = v2.Position.X;
		newTri.Positions[2].y = v2.Position.Y;
		newTri.Positions[2].z = v2.Position.Z;

		newTri.Normals[0].x = v0.Normal.X;
		newTri.Normals[0].y = v0.Normal.Y;
		newTri.Normals[0].z = v0.Normal.Z;
		
		
		newTri.Normals[1].x = v1.Normal.X;
		newTri.Normals[1].y = v1.Normal.Y;
		newTri.Normals[1].z = v1.Normal.Z;
		
		newTri.Normals[2].x = v2.Normal.X;
		newTri.Normals[2].y = v2.Normal.Y;
		newTri.Normals[2].z = v2.Normal.Z;

		//newTri.Normals[0] *= 100;
		//newTri.Normals[1] *= 100;
		//newTri.Normals[2] *= 100;

		newTri.RandomizeColors();
		triangles.push_back(newTri);
	}
	
	while (true)
	{
		screen.Clear();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

		}

		const float scale_factor = 100;
		glm::mat4 transformat = glm::translate(glm::vec3(ScreenWidth/2,ScreenHeight/2, 1.0))*glm::scale(glm::vec3(scale_factor,-scale_factor, scale_factor))* glm::rotate(rot, glm::vec3(0.f, 1.f, 0.f));

		for (auto & t : triangles)
		{
			Triangle newtri = t.GetMultipliedByMatrix(transformat);
			drawTri(newtri, [&](ScreenCoord p, float w0, float w1, float w2)
			{
				glm::vec3 color = w0 * newtri.Normals[0] + w1 * newtri.Normals[1] + w2 * newtri.Normals[2];
				screen.SetPixel(p.x, p.y, Color(color.r, color.g, color.b));
			}
			);
		}
		/*
		Triangle newtri = testTri.GetMultipliedByMatrix(transformat);
		drawTri(newtri, [&](ScreenCoord p, float w0, float w1, float w2)
		{
			glm::vec4 color = w0 * testTri.Colors[0] + w1 * testTri.Colors[1] + w2 * testTri.Colors[2];
			screen.SetPixel(p.x, p.y, Color(color.r, color.g, color.b));
		}
		);*/


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