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
void ParallelForSubfunction(ftl::TaskScheduler *taskScheduler, void *arg) {
	ParallelForTaskData *subset = reinterpret_cast<ParallelForTaskData *>(arg);

	rmt_ScopedCPUSample(Parallel_For_Work, 0);

	for (auto i = subset->begin; i < subset->end; i++)
	{
		subset->kernel(i);
	}
}

TaskEnd<ParallelForTaskData> *Parallel_For(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, std::function<void(size_t)> kernel) {
	
	if ( begin > end || begin == end)
	{
		return nullptr;
	}

	//serial
	if (taskScheduler == nullptr || end-begin < batch)
	{
		rmt_ScopedCPUSample(Parallel_For_Work, 0);

		for (auto i = begin; i < end; i++)
		{
			kernel(i);
		} 

		return nullptr;
	}	
	//go parallel
	else if (taskScheduler)
	{

		TaskEnd<ParallelForTaskData> *taskEnd = new TaskEnd<ParallelForTaskData>(taskScheduler);

		const uint64 numTasks = (end - begin) / batch;
		ftl::Task * tasks = new ftl::Task[numTasks];
		ParallelForTaskData * taskdata = new ParallelForTaskData[numTasks];
		//ftl::AtomicCounter counter(taskScheduler);
		
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
			
			taskScheduler->AddTasks(numTasks, tasks, &taskEnd->counter);

		}
		return taskEnd;
		//taskScheduler->WaitForCounter(&counter, 0);
		//delete[] tasks;
		//delete[] taskdata;
	}

}