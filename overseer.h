/////////////////////////////////////////////////////////////////////////////////////////
//Begin utility_macros.h
#ifdef PTHREAD_TEST_MEM_DEBUG
	//Malloc preprocessor macros taken from https://git.busybox.net/uClibc/tree/libc/stdlib/malloc/malloc.h
	//
	#define MALLOC_ALIGNMENT \
  		(__alignof__ (double) > sizeof (size_t) ? __alignof__ (double) : sizeof (size_t))
	#define MALLOC_HEADER_SIZE				\
  		(MALLOC_ALIGNMENT < sizeof (size_t)	\
   		? sizeof (size_t)					\
   		: MALLOC_ALIGNMENT)

   	#define MALLOC_BASE(addr)	((void *)((char *)addr - MALLOC_HEADER_SIZE))
	#define MALLOC_SIZE(addr)	(*(size_t *)MALLOC_BASE(addr))
	
	void *tracked_malloc(int count, size_t size, int report)
	{
		
		static long num_allocations = 0;
		static long mem_allocated = 0;
		if(!report)
		{
			void *ptr = malloc(size * count);
			mem_allocated += MALLOC_SIZE(ptr);
			num_allocations++;
			
			return ptr;
		}
		else
		{
			printf("allocations | size\n");
			printf(" %10ld | %10ld\n", num_allocations, mem_allocated);
			return NULL;
		}
	}

	void tracked_free(void *ptr, int report)
	{
		static long num_frees = 0;
		static long mem_freed = 0;
		if(!report)
		{
			num_frees++;
			mem_freed += MALLOC_SIZE(ptr);
			free(ptr);
		}
		else
		{
			printf("frees       | size\n");
			printf(" %10ld | %10ld\n", num_frees, mem_freed);
		}
	}

	void tracked_mem_report()
	{
		tracked_malloc(0,0,1);
		tracked_free(NULL, 1);
	}

#endif
#ifndef TASK_POOL_INITIAL_SIZE
	#define TASK_POOL_INITIAL_SIZE 10
#endif
#ifndef CAS
	#define CAS(ptr, old_val, new_val) __sync_bool_compare_and_swap(ptr, old_val, new_val)
#endif
#ifndef MALLOC
	#ifdef PTHREAD_TEST_MEM_DEBUG
		#define MALLOC(TYPE) (TYPE *)tracked_malloc(1, sizeof(TYPE),0)
		#define FREE(ptr) tracked_free(ptr, 0)
	#else
		#define MALLOC(TYPE) (TYPE *)malloc(sizeof(TYPE))
		#define FREE(ptr) free(ptr);
	#endif
#endif
#ifndef MALLOC_MULTIPLE
	#ifdef PTHREAD_TEST_MEM_DEBUG
		#define MALLOC_MULTIPLE(TYPE, COUNT) (TYPE *)tracked_malloc(COUNT, sizeof(TYPE),0)
	#else
		#define MALLOC_MULTIPLE(TYPE, COUNT) (TYPE *)malloc(sizeof(TYPE) * (COUNT))
	#endif
#endif
#ifndef PTR_AS_TYPE
	#define PTR_AS_TYPE(ptr, type) (type *)(ptr)
#endif
#ifndef STRINGIFY
	#define STRINGIFY(s) #s
#endif
//End utility_macros.h
///////////////////////////////////////////////////////////////////////////////////////////////
//Begin Task.h
typedef void* task_argument;
typedef void* task_result;
typedef task_result(*task_function)(task_argument);


typedef void* callback_in;
typedef struct callback_argument
{
	task_result task_out;
	callback_in callback_in;
}callback_argument;

typedef void (*callback_function)(callback_argument *);

typedef struct callback
{
	int set;
	callback_function func;
	callback_argument arg;
}callback;

void callback_process_arg(void *callback_in_ptr, void **callback_in_location, void **task_out_location)
{
	callback_argument *processed_argument = PTR_AS_TYPE(callback_in_ptr, callback_argument);
	*callback_in_location = processed_argument->callback_in;
	*task_out_location = processed_argument->task_out;
}

//uhhhh I'm not passing the task result in anywhere...
//how do I handle that?
void callback_execute(callback in_callback)
{
	if(in_callback.set)
	{
		callback_argument *arg = MALLOC(callback_argument);
		printf("location of callback argument: %p\n", arg);
		*arg = in_callback.arg;
		in_callback.func(arg);
		printf("freeing in_callback arg at %p\n", arg);
		FREE(arg);
	}
}

typedef struct task
{
	int task_id;
	
	void *(*task_func)(void *);
	void *task_argument;

	void *task_result;
	pthread_spinlock_t lock;

	callback on_complete;
}task;

int try_lock_task(task *in_task)
{
	return pthread_spin_trylock(&in_task->lock);
}

void lock_task(task *in_task)
{
	pthread_spin_lock(&in_task->lock);
}

void unlock_task(task *in_task)
{
	pthread_spin_unlock(&in_task->lock);
}

void set_callback(task *in_task, callback in_callback)
{
	printf("Callback set for task: %d\n", in_task->task_id);
	in_task->on_complete = in_callback;
	in_task->on_complete.set = 1;
}

void free_task(task *in_task)
{
	printf("destroying task %d\n", in_task->task_id);
	pthread_spin_destroy(&in_task->lock);
	FREE(in_task);
}

task *create_task(void* (*task_func)(void *), void *task_arg)
{
	task *new_task = MALLOC(task);
	new_task->task_func = task_func;
	new_task->task_argument = task_arg;
	new_task->task_result = NULL;
	pthread_spin_init(&new_task->lock, PTHREAD_PROCESS_PRIVATE);
	return new_task;
}
//END task.h
///////////////////////////////////////////////////////////////////////////////////////////////
//begin concurrent_queue.h

typedef struct concurrent_queue
{
	void **array;
	int count;
	int max;
	pthread_spinlock_t lock;
}concurrent_queue;

int queue_grow(concurrent_queue *queue)
{
	int double_size = queue->max * 2;
	void **double_array = MALLOC_MULTIPLE(void *,double_size);

	if(double_array != NULL)
	{
		void *old_array = queue->array;

		memcpy(double_array, old_array, sizeof(void *) * queue->count);
		queue->max = double_size;
		queue->array = double_array;

		FREE(old_array);
	}
	printf("grow\n");

	return !!double_array;
}

int queue_full(concurrent_queue *queue)
{
	if(queue == NULL)
	{
		printf("queue_full: queue is null pointer\n");
		return -1;
	}
	return queue->count == queue->max;
}

int queue_empty(concurrent_queue *queue)
{
	if(queue == NULL)
	{
		printf("queue_empty: queue is null pointer\n");
		return -1;
	}
	return queue->count == 0;
}

int queue_count(concurrent_queue *queue)
{
	if(queue == NULL)
	{
		printf("queue_count: queue is null pointer\n");
		return -1;
	}
	return queue->count;
}

int queue_push(concurrent_queue *queue, void *data)
{
	int success = 0;
	if(queue == NULL)
	{
		printf("queue_push: queue is null pointer\n");
	}
	else
	{
		pthread_spin_lock(&queue->lock);
		//Cool little bit of unintended first order logic here:  
		//	if P[full] then Q [grow] -> !P [full] || Q [grow] and then the
		//	brackets is the world in which the statement is sound
		//may be obvious but I've never thought of it that way before...
		if(!queue_full(queue) || queue_grow(queue)) 
		{
			queue->array[queue->count++] = data;
			success = 1;
		}
		pthread_spin_unlock(&queue->lock);
	}
	return success; 
}

void *queue_pop(concurrent_queue *queue)
{
	if(queue == NULL)
	{
		printf("queue_pop: queue is null pointer\n");
		return NULL;
	}
	void *result = NULL;
	pthread_spin_lock(&queue->lock);
	if(!queue_empty(queue))
	{
		result = queue->array[--queue->count];
	}
	pthread_spin_unlock(&queue->lock);
	return result;
}

concurrent_queue *create_concurrent_queue(int count)
{
	concurrent_queue *queue = MALLOC(concurrent_queue);
	queue->max = count;
	queue->count = 0;
	queue->array = MALLOC_MULTIPLE(void *,queue->max);
	pthread_spin_init(&queue->lock, PTHREAD_PROCESS_PRIVATE);
	printf("created concurrent queue at address: %p \n", queue);
	return queue;
}

void free_concurrent_queue(concurrent_queue *queue)
{
	pthread_spin_destroy(&queue->lock);
	FREE(queue->array);
	FREE(queue);
}

//end concurrent_queue.h
//////////////////////////////////////////////////////////////////////////////////////////////////
//begin concurrent_counter.h

typedef struct concurrent_counter
{
	int counter;
}concurrent_counter;

void init_concurrent_counter(concurrent_counter *cc)
{
	cc->counter = 0;
}

int increment_counter(concurrent_counter *cc)
{
	int old_val;
	do{
		old_val = cc->counter;
	}while(!CAS(&cc->counter, old_val, old_val+1));
	return old_val+1;
}

int decrement_counter(concurrent_counter *cc)
{
	int old_val;
	do{
		old_val = cc->counter;
	}while(!CAS(&cc->counter, old_val, old_val-1));

	return old_val-1;
}
//end concurrent_counter.h
//////////////////////////////////////////////////////////////////////////////////////////////
//Begin task_board.h

typedef struct task_board
{
	concurrent_counter id_generator;
	concurrent_queue *available_tasks;
	//concurrent_queue *assigned_tasks;	
}task_board;

task_board *create_task_board(int max_tasks)
{
	task_board *new_board = MALLOC(task_board);
	new_board->available_tasks = create_concurrent_queue(max_tasks);
	//new_board->assigned_tasks = create_concurrent_queue(max_tasks);
	init_concurrent_counter(&new_board->id_generator);

	printf("Task board created at location: %p\n", new_board);

	return new_board;
}

void destroy_task_board(task_board *board)
{
	while(!queue_empty(board->available_tasks))
	{
		task *top = queue_pop(board->available_tasks);
		free_task(top);
	}
	free_concurrent_queue(board->available_tasks);
	FREE(board);
}

int push_available(task_board *board, task *new_task)
{
	//printf("beginning push task\n");
	if(board == NULL)
	{
		printf("task board not created\n");
	}
	if(new_task == NULL)
	{
		printf("task not created\n");
	}
	//printf("assigning id\n");
	//printf("location of board: %p\n", board);
	//printf("location of id generator: %p\n", &(board->id_generator));
	new_task->task_id = increment_counter(&board->id_generator);
	//printf("id assigned\n");
	if(queue_push(board->available_tasks, (void *)new_task))
	{
		printf("Successfully pushed task: %d\n", new_task->task_id);
		return new_task->task_id;
	}
	//printf("Failed to push task: %d\n", new_task->task_id);
	return -1;
}

task *pop_available(task_board *board)
{
	task *next;
	if((next = PTR_AS_TYPE(queue_pop(board->available_tasks), task))!=NULL)
	{
		printf("Successfully popped task: %d\n", next->task_id);
		return next;
	}
	//printf("Failed to pop task\n");
	return NULL;
}

int task_count(task_board *board)
{
	return queue_count(board->available_tasks);
}

task *find_task_by_id(task_board *board, int task_id)
{
	int i,task_count = queue_count(board->available_tasks);
	printf("tasks checked:\t ");
	for(i = 0; i < task_count; ++i)
	{
		task *next_task = PTR_AS_TYPE(board->available_tasks->array[i], task);
		printf("%d ", next_task->task_id);
		if(next_task->task_id == task_id)
		{
			printf("\n");
			return next_task;
		}
	}
	printf("\nfailed to find task with id: %d\n", task_id);
	return NULL;
}

//end Task_board.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Begin worker.h
#include "unistd.h"

//worker api struct
typedef enum WORKER_STATUS
{
	WS_UNKNOWN,
	WS_AWAIT,
	WS_ACTIVE,
	WS_BARRIER,
	WS_COMPLETE,
	WS_SLEEP,
	WS_RETIRE
}WORKER_STATUS;

typedef struct worker
{
	int my_id;
	WORKER_STATUS status;
	WORKER_STATUS requested_status;
	task_board *tasks;
	task *my_task;
	pthread_t thread;
	pthread_barrier_t *global_barrier;
}worker;

//exposed functions
void worker_set_status(worker *w, WORKER_STATUS status)
{
	printf("setting worker %d status\n", w->my_id);
	switch(status)
	{
		case WS_ACTIVE:{}break;
		case WS_AWAIT:{
			if(w->requested_status != WS_RETIRE)
			{
				printf("Request Worker %d await next task\n", w->my_id);
				w->requested_status = WS_AWAIT;
			}
		}break;
		case WS_BARRIER:{
			printf("Request Worker %d wait for barrier\n", w->my_id);
			w->requested_status = WS_BARRIER;	
		}break;
		case WS_COMPLETE:{}break;
		case WS_SLEEP:{
			if(w->requested_status != WS_RETIRE)
			{
				printf("Request Worker %d sleep\n", w->my_id);
				w->requested_status = WS_SLEEP;	
			}
		}break;
		case WS_RETIRE:{
			printf("Request Worker %d retire\n", w->my_id);
			w->requested_status = WS_RETIRE;
		}break;
		default:{}break;
	}
}

WORKER_STATUS get_worker_status(worker *w)
{
	if(w == NULL)
	{
		//printf("get_worker_status: invalid worker\n");
		return WS_UNKNOWN;
	}
	return w->status;
}

void execute_task(task *my_task)
{
	lock_task(my_task);
	my_task->task_result = my_task->task_func(my_task->task_argument);
	if(my_task->on_complete.set)
	{
		my_task->on_complete.arg.task_out = my_task->task_result;
	}
	unlock_task(my_task);
}

void free_worker(worker *worker_info)
{
	printf("Worker %d freed\n", worker_info->my_id);
	FREE(worker_info);
}

void *worker_func(void *arg)
{
	worker *worker_info = (worker *)arg;
	while(worker_info->status != WS_RETIRE)
	{
		switch(worker_info->status)
		{
			case WS_AWAIT:{
				task *next_task = NULL;
				if((next_task = pop_available(worker_info->tasks)) != NULL)
				{
					worker_info->status = WS_ACTIVE;
					worker_info->my_task = next_task;
					printf("Worker %d Set Active\n", worker_info->my_id);	
				}
				else{
					if(worker_info->status != worker_info->requested_status)
					{
						printf("worker %d awaiting: now setting status to %d\n", worker_info->my_id, worker_info->requested_status);
						worker_info->status = worker_info->requested_status;
					}
				}
			}break;
			case WS_ACTIVE:{
				printf("Worker %d executing task\n", worker_info->my_id);
				execute_task(worker_info->my_task);
				printf("worker %d task result located at %p\n", worker_info->my_id, worker_info->my_task->task_result);
				worker_info->status = WS_COMPLETE;
				printf("Worker %d Set Complete\n", worker_info->my_id);
			}break;
			case WS_BARRIER:{
				printf("Worker %d At Barrier at location: %p\n", worker_info->my_id, worker_info->global_barrier);
				worker_set_status(worker_info,WS_AWAIT);
				pthread_barrier_wait(worker_info->global_barrier);
				if(worker_info->status != worker_info->requested_status)
				{
					printf("worker %d barrier: now setting status to %d\n", worker_info->my_id, worker_info->requested_status);
					worker_info->status = worker_info->requested_status;
				}
			}break;
			case WS_COMPLETE:{
				printf("Worker %d task finished\n", worker_info->my_id);
				if(worker_info->my_task->on_complete.set)
				{
					printf("Worker %d handling callback\n", worker_info->my_id);
					callback_execute(worker_info->my_task->on_complete);
				}
				printf("worker %d Setting task to null\n", worker_info->my_id);
				free_task(worker_info->my_task);
				worker_info->my_task = NULL;
				if(worker_info->status != worker_info->requested_status)
				{
					printf("worker %d complete: now setting status to %d\n", worker_info->my_id, worker_info->requested_status);
					worker_info->status = worker_info->requested_status;
				}
			}break;
			case WS_SLEEP:{
				sched_yield();
				if(worker_info->status != worker_info->requested_status)
				{
					printf("worker %d sleep: now setting status to %d\n", worker_info->my_id, worker_info->requested_status);
					worker_info->status = worker_info->requested_status;
				}
			}break;
			case WS_RETIRE:{
				printf("Worker %d Set Retire\n", worker_info->my_id);
			}
			default:
			{
			}break;
		}
	}
}

//todo: fix initialization function after getting this working
worker *init_worker(int id, pthread_barrier_t *global_barrier, task_board *tasks)
{
	worker *worker_info = MALLOC(worker);
	worker_info->my_id = id;
	worker_info->status = WS_SLEEP;
	worker_info->requested_status = WS_SLEEP;
	worker_info->my_task = NULL;
	worker_info->tasks = tasks;
	worker_info->global_barrier = global_barrier;
	pthread_create(&worker_info->thread, NULL, worker_func, PTR_AS_TYPE(worker_info, void));
	printf("worker %d created\n", worker_info->my_id);

	return worker_info;
}

//End worker.h
//////////////////////////////////////////////////////////////////////////////////////////////////
//begin worker_pool.h
typedef struct worker_pool
{
	 concurrent_queue *storage;
	 concurrent_counter id_generator;
}worker_pool;

int worker_count(worker_pool *workers)
{
	return queue_count(workers->storage);
}

worker *get_worker_by_id(worker_pool *workers, int id)
{
	int i;
	worker *next_worker;
	for(i = 0; i < queue_count(workers->storage); ++i)
	{
		next_worker = PTR_AS_TYPE(&workers->storage[i], worker);
		if(next_worker->my_id == id)
		{
			return next_worker;
		}
	}
	return NULL;
}

worker *get_worker_by_position(worker_pool *workers, int index)
{
	if(index < queue_count(workers->storage))
	{
		return PTR_AS_TYPE(workers->storage->array[index], worker);
	}
	return NULL;
}

worker_pool *create_worker_pool(int size)
{
	worker_pool *workers = MALLOC(worker_pool);
	workers->storage = create_concurrent_queue(size);
	init_concurrent_counter(&workers->id_generator);
	return workers; 
}

void push_worker(worker_pool *workers, worker *new_worker)
{
	if(!queue_full(workers->storage))
	{
		queue_push(workers->storage, (void *)new_worker);
	}
}

worker *pop_worker(worker_pool *workers)
{
	if(!queue_empty(workers->storage))
	{
		worker *top = PTR_AS_TYPE(queue_pop(workers->storage), worker);
		return top;
	}
	return NULL;
}

void destroy_worker_pool(worker_pool *workers)
{
	while(!queue_empty(workers->storage))
	{
		worker *top = pop_worker(workers);
		free_worker(top);	
	}
	free_concurrent_queue(workers->storage);
	FREE(workers);
}

//end worker_pool.h
//////////////////////////////////////////////////////////////////////////////////////////////////
//begin thread_pool.h

typedef struct thread_pool
{
	task_board *tasks;
	worker_pool *workers;
	pthread_spinlock_t lock;
	pthread_barrier_t global_barrier;
	int main_thread_id;
}thread_pool;

thread_pool *create_thread_pool(int size)
{
	thread_pool *pool = MALLOC(thread_pool);
	pool->tasks = create_task_board(TASK_POOL_INITIAL_SIZE);
	pool->workers = create_worker_pool(size);
	pthread_barrier_init(&pool->global_barrier, NULL, size+1);
	pthread_spin_init(&pool->lock, PTHREAD_PROCESS_PRIVATE);

	int i;
	worker *next_worker;
	for(i = 0; i < size; ++i)
	{
		worker *next_worker = init_worker(increment_counter(&pool->workers->id_generator), &pool->global_barrier, pool->tasks);
		push_worker(pool->workers, next_worker);
	}
	return pool;
}

int disable_workers(thread_pool *pool)
{
	if(pool == NULL)
	{
		return 0;
	}
	pthread_spin_lock(&pool->lock);
	int worker_index,count = worker_count(pool->workers);
	for(worker_index = 0; worker_index < count; ++worker_index)
	{
		worker_set_status(get_worker_by_position(pool->workers, worker_index), WS_SLEEP);
	}
	pthread_spin_unlock(&pool->lock);
	return 1;
}

int enable_workers(thread_pool *pool)
{
	if(pool == NULL)
	{
		return 0;
	}
	pthread_spin_lock(&pool->lock);
	int worker_index,count = worker_count(pool->workers);
	for(worker_index = 0; worker_index < count; ++worker_index)
	{
		worker_set_status(get_worker_by_position(pool->workers, worker_index), WS_AWAIT);
	}
	pthread_spin_unlock(&pool->lock);
	return 1;
}

int insert_task(thread_pool *pool, task_function func, task_argument arg)
{
	if(pool == NULL)
	{
		return -1;
	}
	pthread_spin_lock(&pool->lock);
	task *new_task = create_task(func, arg);
	int task_id = push_available(pool->tasks, new_task);
	pthread_spin_unlock(&pool->lock);

	return task_id;
}

callback create_callback(callback_function func, callback_in arg)
{
	callback new_callback = {0};
	new_callback.set = 1;
	new_callback.func = func;
	new_callback.arg.task_out = NULL;
	new_callback.arg.callback_in = arg;

	return new_callback;
}

int insert_task_with_callback(thread_pool *pool, task_function func, task_argument arg, callback_function callback_func, callback_in callback_arg)
{
	if(pool == NULL)
	{
		return -1;
	}
	pthread_spin_lock(&pool->lock);
	task *new_task = create_task(func, arg);
	set_callback(new_task, create_callback(callback_func, callback_arg));
	printf("location of threads: %p\n",pool);
	printf("location of tasks: %p\n", pool->tasks);
	int task_id = push_available(pool->tasks, new_task);
	pthread_spin_unlock(&pool->lock);
	return task_id;
}

int await_tasks_complete(thread_pool *pool)
{
	if(pool == NULL)
	{
		return 0;
	}
	pthread_spin_lock(&pool->lock);
	int worker_index,count = worker_count(pool->workers);
	while(task_count(pool->tasks) > 0)
	{
		sched_yield();
	}
	for(worker_index = 0; worker_index < count; ++worker_index)
	{
		worker_set_status(get_worker_by_position(pool->workers, worker_index), WS_BARRIER);
	}
	printf("thread pool waiting at barrier at location: %p\n", &pool->global_barrier);
	pthread_barrier_wait(&pool->global_barrier);
	pthread_spin_unlock(&pool->lock);
	return 1;
}

int retire_thread_pool(thread_pool *pool)
{
	if(pool == NULL)
	{
		return 0;
	}
	pthread_spin_lock(&pool->lock);
	int worker_index,count = worker_count(pool->workers);
	for(worker_index = 0; worker_index < count; ++worker_index)
	{
		worker_set_status(get_worker_by_position(pool->workers, worker_index), WS_RETIRE);
	}

	printf("freeing %d workers\n", count);
	for(worker_index = 0; worker_index < count; ++worker_index)
	{
		worker *next_worker = get_worker_by_position(pool->workers, worker_index);
		printf("calling join\n");
		pthread_join(next_worker->thread, NULL);
	}
	destroy_task_board(pool->tasks);
	destroy_worker_pool(pool->workers);

	pthread_spin_unlock(&pool->lock);
	pthread_barrier_destroy(&pool->global_barrier);
	pthread_spin_destroy(&pool->lock);
	
	FREE(pool);
	#ifdef PTHREAD_TEST_MEM_DEBUG
		tracked_mem_report();
	#endif
	return 1;
}