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

#include "concrt.h"

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

		const float scale_factor = 200;
		glm::mat4 transformat = glm::translate(glm::vec3(ScreenWidth / 2, ScreenHeight / 2, -1000.0))*glm::scale(glm::vec3(scale_factor, -scale_factor, scale_factor))* glm::rotate(rot, glm::vec3(0.f, 1.f, 0.f));

		//rmt_ScopedCPUSample(VertexShader, 0);
		VertexShaderInputs vInputs;
		vInputs.InputTriangles = &MeshTriangles;
		vInputs.OutputTriangles = &PostTransformTriangles;
		vInputs.ModelMatrix = &transformat;
		


			auto end = ExecuteVertexShader(taskScheduler, 0, PostTransformTriangles.size(), 4096, &vInputs);
			
			end->Wait(end->numtasks/2,true);

			auto first_par = Parallel_For(//nullptr
				taskScheduler

				, 0, g_Framebuffer->Tiles.size(), 1, [&](auto i) {

				g_Framebuffer->DrawTile(&g_Framebuffer->Tiles[i]);
				
			});
			end->Wait(0, true);
			first_par->Wait(0, true);
			
			
			delete end;

			

			auto second_par = Parallel_For(//nullptr
				taskScheduler

				, 0, g_Framebuffer->Tiles.size(), 1, [&](auto i) {

				g_Framebuffer->DrawTile(&g_Framebuffer->Tiles[i]);
			});

			
			second_par->Wait(0, true);


			

			delete first_par;
			delete second_par;
			
					

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

		}
		auto endt = std::chrono::system_clock::now();
		auto elapsed = endt - start;
		std::cout << "Rasterizer Full Frame Time: " << elapsed.count() << '\n';

		{
			rmt_ScopedCPUSample(Wait, 0);
			//stall
			//Concurrency::wait(500);
		}

		screen.DrawFrame();

		rot += 0.1f;
	}


}
float rot = 0;
Screen*g_Framebuffer{nullptr};
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
	//MeshLoader.LoadFile("Monkey.obj");

	
	

	//load the mesh into internal format
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

	
	{
		screen.Clear();

		
		
		const float scale_factor = 100;
		glm::mat4 transformat = glm::translate(glm::vec3(ScreenWidth/2,ScreenHeight/2, -1000.0))*glm::scale(glm::vec3(scale_factor,-scale_factor, scale_factor))* glm::rotate(rot, glm::vec3(0.f, 1.f, 0.f));
		

		RenderTaskData RenderData;
		RenderData.MeshTriangles = &Dragon.Triangles;
		RenderData.PostTransformTriangles = &PostTransformTriangles;		
		RenderData.transform = &transformat;	


		//run the core rendering task
		taskScheduler.Run(50, RenderMeshTask, &RenderData,0,ftl::EmptyQueueBehavior::Sleep);

		
		
		
	}
	


	// Destroy the main instance of Remotery.
	rmt_DestroyGlobalInstance(rmt);
	return 0;
}