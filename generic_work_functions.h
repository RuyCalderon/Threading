typedef struct array_arg
{
	int *array;
	int arr_size;
}array_arg;

typedef struct parallel_sum_result
{
	int sum;
}parallel_sum_result;

array_arg *create_and_format_array_arg(int *array, int size)
{
	array_arg *work = MALLOC(array_arg);
	work->array = array;
	work->arr_size = size;
	return work;
}

int sum_array(int *array, int array_size)
{
	int sum = 0;
	if(array_size > 0)
	{
		int i;
		sum = array[0];
		for(i = 1; i < array_size;++i)
		{
			sum += array[i];
		}
	}

	return sum;
}

void *parallel_sum(void *argument)
{
	array_arg *args = (array_arg *)argument;

	int sum = sum_array(args->array, args->arr_size);

	FREE(argument);
	
	parallel_sum_result *result = (parallel_sum_result *)malloc(sizeof(parallel_sum_result));
	result->sum = sum;
	
	return (void *)result;
}


typedef struct parallel_min_result
{
	int min;
}parallel_min_result;

int array_min(int *array, int array_size)
{
	int min = 0;
	if(array_size > 0)
	{
		int i;
		min = array[0];
		for(i = 1; i < array_size;++i)
		{
			if(array[i] < min)
			{
				min = array[i];
			}
		}
	}

	return min;
}

void *parallel_min(void *argument)
{
	printf("entering min task\n");
	array_arg *args = (array_arg *)argument;

	int min = array_min(args->array, args->arr_size);

	FREE(argument);
	parallel_min_result *result = (parallel_min_result *)malloc(sizeof(parallel_min_result));
	result->min = min;
	printf("min task finished. Result located at: %p \n", result);
	return (void *)result;
}

void *empty_task(void *argument)
{
	return NULL;
}