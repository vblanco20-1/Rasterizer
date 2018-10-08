#pragma once
#include "RasterizerMath.h"


struct VertexShaderInputs {
	std::vector<Triangle> * InputTriangles;
	std::vector<Triangle> * OutputTriangles;
	glm::mat4 *ModelMatrix;
};

struct VertexShaderTaskData {
	size_t begin;
	size_t end;
	VertexShaderInputs * Inputs;
};

void VertexShaderSubfunction(ftl::TaskScheduler *taskScheduler, void *arg) {
	VertexShaderTaskData *subset = reinterpret_cast<VertexShaderTaskData *>(arg);

	rmt_ScopedCPUSample(VertexShader_Work, 0);

	std::vector<Triangle> & MeshTriangles = *subset->Inputs->InputTriangles;
	std::vector<Triangle> & PostTransformTriangles = *subset->Inputs->OutputTriangles;
	glm::mat4 mat = *subset->Inputs->ModelMatrix;
	size_t i = 0;

	for (i = subset->begin; i < subset->end ; i += 1)
	{
		PostTransformTriangles[i] = MeshTriangles[i].GetMultipliedByMatrix(mat);
	}
}



void ExecuteVertexShader(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, VertexShaderInputs *Inputs) {

	if (begin > end || begin == end)
	{
		return;
	}

	

	//serial
	if (taskScheduler == nullptr || end - begin < batch)
	{
		rmt_ScopedCPUSample(Parallel_For_Work, 0);

		VertexShaderTaskData Task;
		Task.begin = begin;
		Task.end = end;
		Task.Inputs = Inputs;

		VertexShaderSubfunction(nullptr, &Task);
		
	}
	//go parallel
	else if (taskScheduler)
	{


		const uint64 numTasks = (end - begin) / batch;
		ftl::Task * tasks = new ftl::Task[numTasks];
		VertexShaderTaskData * taskdata = new VertexShaderTaskData[numTasks];
		ftl::AtomicCounter counter(taskScheduler);
		{

			rmt_ScopedCPUSample(Parallel_For_Init, 0);
			//prepare tasks
			for (auto i = 0; i < numTasks; i++)
			{
				VertexShaderTaskData * dat = &taskdata[i];
				dat->begin = i * batch;
				dat->end = (i + 1) * batch;
				dat->Inputs = Inputs;
				if (dat->end > end)
				{
					dat->end = end;
				}

				tasks[i] = { VertexShaderSubfunction, dat };
			}

			// Schedule the tasks and wait for them to complete

			taskScheduler->AddTasks(numTasks, tasks, &counter);

		}
		taskScheduler->WaitForCounter(&counter, 0);
		delete[] tasks;
		delete[] taskdata;
	}
}