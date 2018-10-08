#pragma once
#include "RasterizerMath.h"
#include "Globals.h"


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

template<typename T>
struct TaskEnd {
	ftl::Task * tasks{ nullptr };
	T * taskdata{ nullptr };
	ftl::AtomicCounter counter;
	ftl::TaskScheduler *taskScheduler{ nullptr };
	int  numtasks{ 0 };

	TaskEnd(ftl::TaskScheduler * sched) : counter(sched), taskScheduler(sched)
	{

	}

	~TaskEnd()
	{
		Wait(0,true);
		delete[] tasks;
		delete[] taskdata;
	}
	void Wait(int counterval = 0, bool PinThread = false) {

		if (taskScheduler)
		{
			taskScheduler->WaitForCounter(&counter, counterval, PinThread);
		}

	};
	
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
		g_Framebuffer->AddTriangleToTiles(PostTransformTriangles[i]);
	}
}



TaskEnd<VertexShaderTaskData> *ExecuteVertexShader(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, VertexShaderInputs *Inputs) {

	if (begin > end || begin == end)
	{
		return nullptr;
	}

	

	//serial
	if (taskScheduler == nullptr )
	{
		rmt_ScopedCPUSample(Vertex_Shader, 0);

		VertexShaderTaskData Task;
		Task.begin = begin;
		Task.end = end;
		Task.Inputs = Inputs;


		VertexShaderSubfunction(nullptr, &Task);
		
		return nullptr;
	}
	//go parallel
	else if (taskScheduler)
	{
		TaskEnd<VertexShaderTaskData> *taskEnd = new TaskEnd<VertexShaderTaskData>(taskScheduler);

		const uint64 numTasks = (end - begin) / batch;
		ftl::Task * tasks = new ftl::Task[numTasks];
		VertexShaderTaskData * taskdata = new VertexShaderTaskData[numTasks];

		taskEnd->numtasks = numTasks;
		//taskEnd->counter = ftl::AtomicCounter(taskScheduler);
		taskEnd->taskdata = taskdata;
		taskEnd->taskScheduler = taskScheduler;
		taskEnd->tasks = tasks;
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

			taskScheduler->AddTasks(numTasks, tasks, &taskEnd->counter);

		}

		return taskEnd;
		//taskEnd.counter = counter;
		//taskEnd.
		//
		//taskScheduler->WaitForCounter(&counter, 0);
		//delete[] tasks;
		//delete[] taskdata;
	}
}