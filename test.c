#include <malloc.h>
#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#define PTHREAD_TEST_MEM_DEBUG
#include "overseer.h"
#include "test_functions.c"

#define Ki(N) (1024 * (N))
#define Mi(N) (1024 * Ki(N))
#define Bi(N) (1024 * Mi(N))

void print_min(void *callback_in_ptr)
{
	printf("in print_min callback\n");
	printf("location of callback_in_ptr: %p\n", callback_in_ptr);
	void *task_out;
	void *callback_in;
	callback_process_arg(callback_in_ptr, &callback_in, &task_out);

	parallel_min_result *task_output = PTR_AS_TYPE(task_out, parallel_min_result);
	printf("location of task out: %p\n", task_output);
	printf("location of callback_in: %p\n", callback_in);
	printf("task %d result: min is: %d\n", *PTR_AS_TYPE(callback_in, int), task_output->min);
}

//turn this into a macro of some kind?
void print_sum(void *callback_in_ptr)
{
	printf("in print_sum callback\n");
	callback_argument *callback_arg = PTR_AS_TYPE(callback_in_ptr, callback_argument);
	
	parallel_sum_result *result = PTR_AS_TYPE(callback_arg->task_out, parallel_sum_result);
	int *task_id = PTR_AS_TYPE(callback_arg->callback_in, int);
	printf("task %d result: sum is: %d\n", *task_id, result->sum);
}

void threads_test(thread_pool *threads)
{
	printf("Beginning test\n");
	int i;
	int count = Mi(128);
	int *sum_array = (int *)malloc(sizeof(int) * count);
	for(i = 0; i < count; ++i)
	{
		sum_array[i] = 1;
	}
	printf("location of threads: %p\n",threads);
	int task_ids[20];
	printf("task ids are located at: %p\n", task_ids);
	for(i = 0; i < 10; ++i)
	{
		int chunk_size = count/20;
		array_arg *work = MALLOC(array_arg);
		work->array = sum_array+i*chunk_size;
		work->arr_size = chunk_size;
		if(i%2 == 0)
		{
			task_ids[i] = insert_task_with_callback(threads, parallel_sum, work, (callback_function)print_sum, (callback_in)&task_ids[i]);
		}
		else
		{
			task_ids[i] = insert_task_with_callback(threads, parallel_min, work, (callback_function)print_min, (callback_in)&task_ids[i]);
		}
	}
	printf("inserted %d tasks\n", 10);
	
	printf("print sum at %p\n", (void *)print_sum);
	printf("print min at %p\n", (void *)print_min);
	
	enable_workers(threads);
	
	for(i = 10; i < 20; ++i)
	{
		int chunk_size = count/20;
		array_arg *work = MALLOC(array_arg);
		work->array = sum_array+i*chunk_size;
		work->arr_size = chunk_size;
		if(i%2 == 0)
		{
			task_ids[i] = insert_task_with_callback(threads, parallel_sum, work, (callback_function)print_sum, (callback_in)&task_ids[i]);
		}
		else
		{
			task_ids[i] = insert_task_with_callback(threads, parallel_min, work, (callback_function)print_min, (callback_in)&task_ids[i]);
		}
	}

	//need to figure out how to do this, like have an active jobs board or something (heyyyy similar to what was there before, right?)
	await_tasks_complete(threads);
	printf("All Threads complete, ending test\n");

	free(sum_array);
}

int main(int argc, char **argv)
{
	thread_pool *threads = create_thread_pool(2);
	
	threads_test(threads);
	
	retire_thread_pool(threads);
	return 0;
}
