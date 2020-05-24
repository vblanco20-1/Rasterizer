#include "ParallelFor.h"
#include "Tracy.hpp"
void ParallelForSubfunction(ftl::TaskScheduler *taskScheduler, void *arg) {
	ParallelForTaskData *subset = reinterpret_cast<ParallelForTaskData *>(arg);

	//rmt_ScopedCPUSample(Parallel_For_Work, 0);
	ZoneScoped;
	for (auto i = subset->begin; i < subset->end; i++)
	{
		subset->kernel(i);
	}
}

void ParallelForSubfunction_Counter(ftl::TaskScheduler *taskScheduler, void *arg) {
	ParallelForTaskData_Wait *subset = reinterpret_cast<ParallelForTaskData_Wait *>(arg);

	{
		//rmt_ScopedCPUSample(Parallel_For_Work_Pre, 0);
		ZoneScoped;
		for (auto i = subset->begin; i < subset->end; i++)
		{
			subset->start_kernel(i);
		}
	}
	taskScheduler->WaitForCounter(subset->counter, 0);
	{
		ZoneScoped;
		//rmt_ScopedCPUSample(Parallel_For_Work_Post, 0);
		for (auto i = subset->begin; i < subset->end; i++)
		{
			subset->end_kernel(i);
		}
	}
}

TaskEnd<ParallelForTaskData> * Parallel_For(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, std::function<void(size_t)> kernel)
{

	if (begin > end || begin == end)
	{
		return nullptr;
	}

	//serial
	if (taskScheduler == nullptr || end - begin < batch)
	{
		ZoneScoped;
		//rmt_ScopedCPUSample(Parallel_For_Work, 0);

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
			ZoneScoped;
			//rmt_ScopedCPUSample(Parallel_For_Init, 0);
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

			g_taskScheduler->AddTasks(numTasks, tasks, &taskEnd->counter);

		}
		return taskEnd;
	}
	return nullptr;
}

TaskEnd<ParallelForTaskData_Wait> * Parallel_For_Counter(ftl::TaskScheduler *taskScheduler, size_t begin, size_t end, size_t batch, ftl::AtomicCounter * counter, std::function<void(size_t)> start_kernel, std::function<void(size_t)> end_kernel)
{

	if (begin > end || begin == end)
	{
		return nullptr;
	}	
	//go parallel
	else if (taskScheduler)
	{

		TaskEnd<ParallelForTaskData_Wait> *taskEnd = new TaskEnd<ParallelForTaskData_Wait>(taskScheduler);

		const uint64 numTasks = (end - begin) / batch;
		ftl::Task * tasks = new ftl::Task[numTasks];
		ParallelForTaskData_Wait * taskdata = new ParallelForTaskData_Wait[numTasks];
		//ftl::AtomicCounter counter(taskScheduler);

		taskEnd->numtasks = numTasks;
		//taskEnd->counter = ftl::AtomicCounter(taskScheduler);
		taskEnd->taskdata = taskdata;
		taskEnd->taskScheduler = taskScheduler;
		taskEnd->tasks = tasks;
		{
			ZoneScoped;
			//rmt_ScopedCPUSample(Parallel_For_Init, 0);
			//prepare tasks
			for (auto i = 0; i < numTasks; i++)
			{
				ParallelForTaskData_Wait * dat = &taskdata[i];
				dat->begin = i * batch;
				dat->end = (i + 1) * batch;
				dat->start_kernel = start_kernel;
				dat->end_kernel = end_kernel;
				dat->counter = counter;
				if (dat->end > end)
				{
					dat->end = end;
				}

				tasks[i] = { ParallelForSubfunction_Counter, dat };
			}

			// Schedule the tasks and wait for them to complete

			g_taskScheduler->AddTasks(numTasks, tasks, &taskEnd->counter);

		}
		return taskEnd;
	}
	return nullptr;
}

