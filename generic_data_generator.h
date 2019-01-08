#define MALLOC_INT_ARR(size) (int *)malloc(sizeof(int) * size)
#define LARGE_ARRAY_SIZE (1024*1024)
#define SMALL_ARRAY_SIZE 1024
#define TINY_ARRAY_SIZE 1

//alot of duplicated code here, but I like having the function call be explicit
//for each size of array instead of having the user have to use an enum or macro
//themselves. Plus, this also allows a user to potentially set how big large,small
//and tiny arrays are from the command line when compiling
 
int *get_int_array_large(int *out_size)
{
	int *arr = MALLOC_INT_ARR(LARGE_ARRAY_SIZE);
	*out_size = LARGE_ARRAY_SIZE;

	return arr;
}
int *get_int_array_small(int *out_size)
{
	int *arr = MALLOC_INT_ARR(SMALL_ARRAY_SIZE);
	*out_size = SMALL_ARRAY_SIZE;

	return arr;
}
int *get_int_array_tiny(int *out_size)
{
	int *arr = MALLOC_INT_ARR(TINY_ARRAY_SIZE);
	*out_size = TINY_ARRAY_SIZE;

	return arr;
}
void free_int_array(int *array)
{
	free(array);
}