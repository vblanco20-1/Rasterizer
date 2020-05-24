#pragma once
#include "RasterizerMath.h"
#include "Globals.h"
#include "Remotery.h"
#include "Screen.h"
#include "ftl/atomic_counter.h"
#include "ftl/task_scheduler.h"


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




TaskEnd<VertexShaderTaskData> *ExecuteVertexShader(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, VertexShaderInputs *Inputs);