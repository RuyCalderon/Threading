#include "generic_work_functions.h"
#include "generic_data_generator.h"
#include "generic_callback_functions.h"

#include <limits.h> // FLT_MAX

typedef int(*thread_test)(thread_pool *t, int *tests_run);
#define TEST_SUCCESS 1
#define TEST_FAIL 0
#define TEST_EXPECT_GT_ZERO INT_MAX


void run_test(thread_test test, thread_pool *pool)
{
	int total = 0;
	int successes = test(pool, &total);
	printf("test completed: %d/%d tests passed\n\n", successes, total);
}

int satisfies_condition(int expected, int actual)
{
	if(expected == TEST_EXPECT_GT_ZERO)
	{
		return actual > 0;
	}
	return expected == actual;
}

int result_expect_int(int expected, int actual, const char *func_name, const char *msg_on_failure, int *out_success_counter, int *out_total_counter)
{
	*out_total_counter++;
	if(satisfies_condition(expected, actual))
	{
		*out_success_counter++;
	}
	else
	{
		printf("%s\n",msg_on_failure);
	}
}

//Error handling
int test_null_pool(thread_pool *t, int *battery_test_count)
{
	printf("Executing Null Pool Tests:\n\n");
	const char *test_battery_identifier = __func__;
	int array_size_out, task_id;
	int *placeholder_array = get_int_array_tiny(&array_size_out);
	
	int succeeded = 0, total = 0;
	array_arg *test_work = create_and_format_array_arg(placeholder_array, array_size_out);
	
	printf("Insert Task:\n");
	result_expect_int(-1, insert_task(t, parallel_sum, test_work), 
		test_battery_identifier, "failed to report error on insert_task", &succeeded, &total);	
	
	printf("Insert Task With Callback:\n");
	result_expect_int(-1, insert_task_with_callback(t, parallel_sum, test_work, (callback_function)heartbeat, NULL), 
		test_battery_identifier, "failed to report error on insert_task_with_callback", &succeeded, &total);
	
	printf("Enable Workers:\n");
	result_expect_int(0, enable_workers(t),
		test_battery_identifier, "failed to report error on enable_workers", &succeeded, &total);
	
	printf("Disable Workers:\n");
	result_expect_int(0, disable_workers(t), 
		test_battery_identifier, "failed to report error on disable_workers", &succeeded, &total);

	printf("Await Tasks Complete:\n");
	result_expect_int(0, await_tasks_complete(t),
		test_battery_identifier, "failed to report error on await_tasks_complete", &succeeded, &total);
	
	printf("Retire Thread Pool:\n");
	result_expect_int(0, retire_thread_pool(t), 
		test_battery_identifier, "failed to report error on retire_thread_pool", &succeeded, &total);

	free(test_work);
	*battery_test_count = total;
	return succeeded;
}

int test_null_task(thread_pool *t, int *battery_test_count)
{
	printf("Executing Null Task Tests:\n\n");
	const char *test_battery_identifier = __func__;
	int array_size_out, task_id;
	int *placeholder_array = get_int_array_tiny(&array_size_out);
	
	int succeeded = 0, total = 0;
	array_arg *test_work = create_and_format_array_arg(placeholder_array, array_size_out);
	
	printf("Insert Task:\n");
	result_expect_int(-1, insert_task(t, parallel_sum, test_work), 
		test_battery_identifier, "failed to report error on insert_task", &succeeded, &total);	
	
	printf("Insert Task With Callback:\n");
	result_expect_int(-1, insert_task_with_callback(t, parallel_sum, test_work, (callback_function)heartbeat, NULL), 
		test_battery_identifier, "failed to report error on insert_task_with_callback", &succeeded, &total);

	free(test_work);
	*battery_test_count = total;
	return succeeded;
}


int test_freed_data(thread_pool *t)
{

}

int test_null_callback(thread_pool *t)
{

}

//incorrect usage
int test_access_task_data(thread_pool *t)
{

}

int test_same_task_assigned_twice(thread_pool *t)
{

}

int test_wrong_type(thread_pool *t)
{

}

//simple tests
int test_no_task_data(thread_pool *t, int *battery_test_count)
{
	const char *test_battery_identifier = __func__;
	int array_size_out, task_id;
	int *placeholder_array = get_int_array_tiny(&array_size_out);
	
	int succeeded = 0, total = 0;
	array_arg *test_work = create_and_format_array_arg(placeholder_array, array_size_out);
	
	printf("Insert Task:\n");
	result_expect_int(TEST_EXPECT_GT_ZERO, insert_task(t, empty_task, NULL), 
		test_battery_identifier, "failed to report error on insert_task", &succeeded, &total);	
	
	printf("Insert Task With Callback:\n");
	result_expect_int(TEST_EXPECT_GT_ZERO, insert_task_with_callback(t, parallel_sum, test_work, (callback_function)heartbeat, NULL), 
		test_battery_identifier, "failed to report error on insert_task_with_callback", &succeeded, &total);

	free(test_work);
	*battery_test_count = total;
	return succeeded;
}

int test_no_callback(thread_pool *t)
{

}
int test_single_task(thread_pool *t)
{

}

int test_multiple_tasks(thread_pool *t)
{

}

int test_different_tasks(thread_pool *t)
{

}

//stress tests

int test_continual_assignment(thread_pool *t)
{

}

int test_many_small_tasks(thread_pool *t)
{

}
