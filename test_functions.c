typedef struct array_arg
{
	int *array;
	int arr_size;
}array_arg;

typedef struct parallel_sum_result
{
	int sum;
}parallel_sum_result;

void *parallel_sum(void *argument)
{
	printf("entering sum task\n");
	array_arg *args = (array_arg *)argument;

	int size = args->arr_size;
	int *array = args->array;
	int sum = array[0];
	int i;
	printf("initialized sum task parameters\n");
	for(i = 1; i < size;++i)
	{
		sum += array[i];
	}
	FREE(argument);
	parallel_sum_result *result = (parallel_sum_result *)malloc(sizeof(parallel_sum_result));
	result->sum = sum;
	printf("sum task finished. Result located at: %p \n", result);
	return (void *)result;
}

typedef struct parallel_min_result
{
	int min;
}parallel_min_result;

void *parallel_min(void *argument)
{
	printf("entering min task\n");
	array_arg *args = (array_arg *)argument;

	int size = args->arr_size;
	int *array = args->array;
	int min = array[0];
	int i;
	printf("initialized min task parameters\n");
	for(i = 1; i < size;++i)
	{
		if(array[i] < min)
		{
			min = array[i];
		}
	}
	FREE(argument);
	parallel_min_result *result = (parallel_min_result *)malloc(sizeof(parallel_min_result));
	result->min = min;
	printf("min task finished. Result located at: %p \n", result);
	return (void *)result;
}