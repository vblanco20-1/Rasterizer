#pragma once
#include <functional>
#include "ftl/atomic_counter.h"
#include "ftl/task_scheduler.h"
#include "VertexShader.h"

struct ParallelForTaskData {
	size_t begin; 
	size_t end; 
	//size_t batch; 
	std::function<void(size_t)> kernel;
};

struct ParallelForTaskData_Wait {
	size_t begin;
	size_t end;
	//size_t batch; 
	std::function<void(size_t)> start_kernel;

	ftl::AtomicCounter * counter;

	std::function<void(size_t)> end_kernel;
};



TaskEnd<ParallelForTaskData> *Parallel_For(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, std::function<void(size_t)> kernel);

TaskEnd<ParallelForTaskData_Wait> *Parallel_For_Counter(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, ftl::AtomicCounter * counter,std::function<void(size_t)> start_kernel, std::function<void(size_t)> end_kernel);