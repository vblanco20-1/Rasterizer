#pragma once
#include <functional>
#include "ftl/atomic_counter.h"
#include "ftl/task_scheduler.h"

struct ParallelForTaskData {
	size_t begin; 
	size_t end; 
	//size_t batch; 
	std::function<void(size_t)> kernel;
};
void ParallelForSubfunction(ftl::TaskScheduler *taskScheduler, void *arg) {
	ParallelForTaskData *subset = reinterpret_cast<ParallelForTaskData *>(arg);

	rmt_ScopedCPUSample(Parallel_For_Work, 0);

	for (auto i = subset->begin; i < subset->end; i++)
	{
		subset->kernel(i);
	}
}

void Parallel_For(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, std::function<void(size_t)> kernel) {
	
	if ( begin > end || begin == end)
	{
		return;
	}

	//serial
	if (taskScheduler == nullptr || end-begin < batch)
	{
		rmt_ScopedCPUSample(Parallel_For_Work, 0);

		for (auto i = begin; i < end; i++)
		{
			kernel(i);
		} 
	}	
	//go parallel
	else if (taskScheduler)
	{

		
		const uint64 numTasks = (end - begin) / batch;
		ftl::Task * tasks = new ftl::Task[numTasks];
		ParallelForTaskData * taskdata = new ParallelForTaskData[numTasks];
		ftl::AtomicCounter counter(taskScheduler);
		{

			rmt_ScopedCPUSample(Parallel_For_Init, 0);
			//prepare tasks
			for (auto i = 0; i < numTasks; i++)
			{
				ParallelForTaskData * dat = &taskdata[i];
				dat->begin = i * batch;
				dat->end = (i + 1) * batch;
				dat->kernel = kernel;
				if (dat->end > end)
				{
					dat->end = end;
				}

				tasks[i] = { ParallelForSubfunction, dat };
			}

			// Schedule the tasks and wait for them to complete
			
			taskScheduler->AddTasks(numTasks, tasks, &counter);

		}
		taskScheduler->WaitForCounter(&counter, 0);
		delete[] tasks;
		delete[] taskdata;
	}

}