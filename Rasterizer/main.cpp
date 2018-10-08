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
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#define OBJL_CONSOLE_OUTPUT
#include "OBJ_Loader.h"
#include <glm/gtx/transform.hpp>
#include <glm/geometric.hpp>

#include "Remotery.h"

#include "ParallelFor.h"
#include "Globals.h"
#include "VertexShader.h"

struct Mesh {
	std::vector<Triangle> Triangles;
};

struct RenderTaskData {
	
	std::vector<Triangle>* MeshTriangles;
	std::vector<Triangle>* PostTransformTriangles;
	glm::mat4 * transform;
};

void RenderMeshTask(ftl::TaskScheduler *taskScheduler, void *arg) {

	
	float rot = 0;
	RenderTaskData*renderdata = reinterpret_cast<RenderTaskData *>(arg);


	Screen&screen = *g_Framebuffer;
	std::vector<Triangle> & MeshTriangles = *renderdata->MeshTriangles;
	std::vector<Triangle> & PostTransformTriangles = *renderdata->PostTransformTriangles;

	while (true)
	{
		
		screen.Clear();

		auto start = std::chrono::system_clock::now();

		const float scale_factor = 100;
		glm::mat4 transformat = glm::translate(glm::vec3(ScreenWidth / 2, ScreenHeight / 2, -1000.0))*glm::scale(glm::vec3(scale_factor, -scale_factor, scale_factor))* glm::rotate(rot, glm::vec3(0.f, 1.f, 0.f));

		//rmt_ScopedCPUSample(VertexShader, 0);
		VertexShaderInputs vInputs;
		vInputs.InputTriangles = &MeshTriangles;
		vInputs.OutputTriangles = &PostTransformTriangles;
		vInputs.ModelMatrix = &transformat;
		{
			

			ExecuteVertexShader(taskScheduler,0, PostTransformTriangles.size(), 4096, &vInputs);
			/*
			Parallel_For(//nullptr
				taskScheduler				 		
				
				,0, PostTransformTriangles.size(), 4000, [&](auto i) {


				PostTransformTriangles[i] = MeshTriangles[i].GetMultipliedByMatrix(transformat);
			});*/
		}




		{
			rmt_ScopedCPUSample(Rasterizer, 0);

			for (Triangle & t : *vInputs.OutputTriangles)
			{
				//Triangle newtri 
				drawTri(t, [](ScreenCoord p,const Triangle& tri ,float w0, float w1, float w2)
				{
					glm::vec3 color = w0 * tri.Normals[0] + w1 * tri.Normals[1] + w2 * tri.Normals[2];
					float depth = 1.0 / (w0 * tri.Positions[0].z + w1 * tri.Positions[1].z + w2 * tri.Positions[2].z);
					g_Framebuffer->SetPixel(p.x, p.y, Color(color.r, color.g, color.b), depth);
				}
				);
			}
		}

	

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

		}
		auto end = std::chrono::system_clock::now();
		auto elapsed = end - start;
		std::cout <<"Rasterizer Full Frame Time: " <<elapsed.count() << '\n';

		screen.DrawFrame();

		rot += 0.1f;
	}


}
float rot = 0;
int main(int argc, char* args[])
{

	ftl::TaskScheduler taskScheduler;

	Remotery* rmt;
	rmt_CreateGlobalInstance(&rmt);
	Screen screen;

	g_Framebuffer = &screen;

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

	testTri.Positions[0].w = 1;
	testTri.Positions[1].w = 1;
	testTri.Positions[2].w = 1;
	testTri.RandomizeColors();
	
	

	
	std::vector<Triangle> triangles;
	auto & vertices = (MeshLoader.LoadedMeshes[0].Vertices);
	auto & indices =  (MeshLoader.LoadedMeshes[0].Indices);
	Mesh Dragon;
	{
		rmt_ScopedCPUSample(MeshLoading, 0);


		for (int i = 0; i < indices.size(); i += 3)
		{
			objl::Vertex v0 = vertices[indices[i]];
			objl::Vertex v1 = vertices[indices[i + 2]];
			objl::Vertex v2 = vertices[indices[i + 1]];

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

			newTri.Positions[0].w = 1;
			newTri.Positions[1].w = 1;
			newTri.Positions[2].w = 1;

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

		Dragon.Triangles = triangles;
	}

	
	MeshLoader.~Loader();
	
	std::vector<Triangle> PostTransformTriangles(Dragon.Triangles.size(),Triangle());

	//while (true)
	{
		screen.Clear();

		
		
		const float scale_factor = 100;
		glm::mat4 transformat = glm::translate(glm::vec3(ScreenWidth/2,ScreenHeight/2, -1000.0))*glm::scale(glm::vec3(scale_factor,-scale_factor, scale_factor))* glm::rotate(rot, glm::vec3(0.f, 1.f, 0.f));
		

		RenderTaskData RenderData;
		RenderData.MeshTriangles = &Dragon.Triangles;
		RenderData.PostTransformTriangles = &PostTransformTriangles;		
		RenderData.transform = &transformat;	

		taskScheduler.Run(50, RenderMeshTask, &RenderData,0,ftl::EmptyQueueBehavior::Sleep);

		
		while (true)
		{

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{

			}
		screen.DrawFrame();
		}
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


	// Destroy the main instance of Remotery.
	rmt_DestroyGlobalInstance(rmt);
	return 0;
}