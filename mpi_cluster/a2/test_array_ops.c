/**
 * Template C file.
 */
/********************* Header Files ***********************/
/* C Headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* Project Headers */
#include "CUnit/Basic.h"
#include "mpi.h"
#include "array_ops.h"

/******************* Constants/Macros *********************/
#define TEMP_FILE 		"temp.txt"
#define VALS_SIZE		20

/******************* Type Definitions *********************/


/**************** Static Data Definitions *****************/
static int vals_orig[] = {62, 58, 41, 85, 39, 10, 64, 69, 41, 5, 98, 27, 2, 97, 30, 22, 39, 94, 56, 21};
static int vals[VALS_SIZE];

/****************** Static Functions **********************/


/**************** Global Data Definitions *****************/


/****************** Global Functions **********************/
/*
 * Suite initialization function run before each test.
 */
int suite_init(void) {
    srand(time(NULL));

    return 0;
}

/* The suite cleanup function.
 */
int suite_clean(void) {

    return 0;
}

/*
 * Compare function, equal case.
 */
void test_compare_equal(void) {
    int a = 9, b = 9;
    CU_ASSERT(lib_compare(&a, &b) == 0);
}

/*
 * Compare function, a less than b case.
 */
void test_compare_less(void) {
    int a = 7, b = 9;
    CU_ASSERT(lib_compare(&a, &b) < 0);
}

/*
 * Compare function, a more than b case.
 */
void test_compare_more(void) {
    int a = 20, b = 9;
    CU_ASSERT(lib_compare(&a, &b) > 0);
}

/*
 * Test pivot selection, case where numbers are odd.
 */
void test_select_pivot_odd(void) {
    int test_data[] = {5, 9, 22, 99, 33}, expected = 22;
    int selected = lib_select_pivot(test_data, sizeof(test_data)/sizeof(int));

    CU_ASSERT(selected == expected);
}

/*
 * Test pivot selection, case where numbers are even.
 */
void test_select_pivot_even(void) {
    int test_data[] = {5, 9, 22, 99, 33, 2}, expected = 22;
    int selected = lib_select_pivot(test_data, sizeof(test_data)/sizeof(int));

    CU_ASSERT(selected == expected);
}

/*
 * Test simple swap function.
 */
void test_swap_ints(void) {
    int orig_a = 10, orig_b = 20, a = orig_a, b = orig_b;

    lib_swap(&a, &b);
    CU_ASSERT(a == orig_b);
    CU_ASSERT(b == orig_a);
}

/*
 * Test power function when exponent is even.
 */
void test_power_even(void) {
    int base = 2, exp = 10, expected = 1024, actual;
    actual = lib_power(base, exp);

    CU_ASSERT(actual == expected);
}

/*
 * Test power function when exponent is odd.
 */
void test_power_odd(void) {
    int base = 2, exp = 7, expected = 128;
    int actual = lib_power(base, exp);

    CU_ASSERT(actual == expected);
}

/*
 * Test partitioning of array based on a passed in pivot value.
 */
void test_partition_by_val(void) {
    int pivot = 40, size = 10, orig_data[] = {99, 33, 10, 77, 900, 88, 61, 22, 33, 100};
    int expected_data[] = {33, 33, 10, 22, 900, 88, 61, 77, 99, 100};
    int actual_lt = 0, actual_gt = 0, expected_lt = 4, expected_gt = 6;

    lib_partition_by_pivot_val(pivot, orig_data, size, &actual_lt, &actual_gt);

    CU_ASSERT(actual_lt == expected_lt)
    CU_ASSERT(actual_gt == expected_gt);

    for (int i = 0; i < size; ++i)
        CU_ASSERT(orig_data[i] == expected_data[i]);
}

/*
 * Test array union, takes two arrays and put them into one larger merged array.
 */
void test_array_union(void) {
    int *left, *right, left_size = 10, right_size = 10;
    int left_data[] = {40, 48, 21, 76, 4, 50, 53, 87, 12, 94};
    int right_data[] = {52, 83, 88, 95, 59, 78, 9, 68, 61, 96};
    int expected_size = left_size+right_size;
    int expected[] = {40, 48, 21, 76, 4, 50, 53, 87, 12, 94,
            52, 83, 88, 95, 59, 78, 9, 68, 61, 96};

    /* Allocate arrays then fill with test data. */
    left = malloc(left_size * sizeof(int));
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(left, NULL);

    right = malloc(right_size * sizeof(int));
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(right, NULL);

    /* Put values in malloced arrays then call union. */
    memcpy(left, left_data, left_size * sizeof(int));
    memcpy(right, right_data, right_size * sizeof(int));
    lib_array_union(&left, &left_size, right, right_size);

    CU_ASSERT(left_size == expected_size);
    for (int i = 0; i < expected_size; ++i)
        CU_ASSERT(left[i] == expected[i]);

    /* Fatal assert above prevents calling if the malloc failed. */
    free(left);
    free(right);
}

/*
 * Test to exercise the array tracing function.
 */
void test_subgroup_info(void) {
    int dimension = 2, id = 6;
    subgroup_info_t actual = {0, 0, 0 ,0, id}, expected = {4, 1, 2, 4, id};

    lib_subgroup_info(dimension, &actual);

    CU_ASSERT(actual.group_num == expected.group_num);
    CU_ASSERT(actual.member_num == expected.member_num);
    CU_ASSERT(actual.partner == expected.partner);
    CU_ASSERT(actual.group_size == expected.group_size);
}

/*
 * Test to exercise the array compressing function.
 */
void test_compress_array(void) {
    int world = 3, vals_size = 60, vals[] = {1, 2, 4, 4, 4, 10, 11, 11, 13, 13, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16, 18, 18, 18, 20, 21, 24, 25, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 27, 28, 28, 28, 31, 32, 33, 33, 34, 34, 35, 37, 39, 39, 43, 45, -1, -1, -1, -1};

    lib_compress_array(world, vals, vals_size);

    for (int i = 0; i < vals_size/2; ++i)
        CU_ASSERT(vals[i] != -1);
}

/*
 * Test the partitioning of the array when pivot is first element.
 */
void test_partition_by_index_first(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_pos = 13, e_vals[] = {30, 58, 41, 21, 39, 10, 56, 39, 41, 5, 22, 27, 2, 62, 97, 98, 69, 94, 64, 85};
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    pos = lib_partition_by_pivot_index(0, vals, vals+VALS_SIZE-1);

    CU_ASSERT(pos == e_pos);
    for (int i = 0; i < VALS_SIZE; ++i)
        CU_ASSERT(vals[i] == e_vals[i]);
}

/*
 * Test the partitioning of the array when pivot is the middle-ish element.
 */
void test_partition_by_index_mid(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_pos = 10, e_vals[] = {27, 21, 41, 39, 39, 10, 22, 30, 2, 5, 41, 98, 62, 97, 69, 64, 85, 94, 56, 58};
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    pos = lib_partition_by_pivot_index(8, vals, vals+VALS_SIZE-1);

    CU_ASSERT(pos == e_pos);
    for (int i = 0; i < VALS_SIZE; ++i)
        CU_ASSERT(vals[i] == e_vals[i]);
}

/*
 * Test the partitioning of the array when pivot is last element.
 */
void test_partition_by_index_last(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_pos = 3, e_vals[] = {10, 2, 5, 21, 39, 85, 64, 69, 41, 41, 98, 27, 58, 97, 30, 22, 39, 94, 56, 62};
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    pos = lib_partition_by_pivot_index(VALS_SIZE-1, vals, vals+VALS_SIZE-1);

    CU_ASSERT(pos == e_pos);
    for (int i = 0; i < VALS_SIZE; ++i)
        CU_ASSERT(vals[i] == e_vals[i]);
}

/*
 * Test the partitioning of the array when pivot is the smallest value.
 */
void test_partition_by_index_smallest_val(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_pos = 0, e_vals[] = {2, 58, 41, 85, 39, 10, 64, 69, 41, 5, 98, 27, 62, 97, 30, 22, 39, 94, 56, 21};
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    pos = lib_partition_by_pivot_index(12, vals, vals+VALS_SIZE-1);

    CU_ASSERT(pos == e_pos);
    for (int i = 0; i < VALS_SIZE; ++i)
        CU_ASSERT(vals[i] == e_vals[i]);
}

/*
 * Test the partitioning of the array when pivot is the largest value.
 */
void test_partition_by_index_largest_val(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_pos = 19, e_vals[] = {21, 58, 41, 85, 39, 10, 64, 69, 41, 5, 62, 27, 2, 97, 30, 22, 39, 94, 56, 98};
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    pos = lib_partition_by_pivot_index(10, vals, vals+VALS_SIZE-1);

    CU_ASSERT(pos == e_pos);
    for (int i = 0; i < VALS_SIZE; ++i)
        CU_ASSERT(vals[i] == e_vals[i]);
}

/*
 * Test the partitioning of the array when pivot is first element.
 */
void test_select_kth(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_val = 41, e_pos = 9;
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    /* Get the smallest value. */
    pos = lib_select_kth(10, vals, vals+VALS_SIZE-1);

    CU_ASSERT(vals[pos] == e_val);
    CU_ASSERT(pos == e_pos);
}

/*
 * Test the partitioning of the array when pivot is first element.
 */
void test_median_of_medians(void) {
    /* Expected position and values after partitioning. */
    int pos = 0, e_val = 41, e_pos = 2;
    memcpy(vals, vals_orig, VALS_SIZE*sizeof(int));

    /* Get the smallest value. */
    pos = lib_median_of_medians(vals, 0, 19);

    CU_ASSERT(vals[pos] == e_val);
    CU_ASSERT(pos == e_pos);
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main() {
   CU_pSuite sharedSuite = NULL;

   /* Initialize the CUnit test registry. */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* Add a suite to the registry. */
   sharedSuite = CU_add_suite("Shared Suite", suite_init, suite_clean);
   if (NULL == sharedSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add the tests to the suite. */
   if (
       (NULL == CU_add_test(sharedSuite, "Compare: Equal..............", test_compare_equal)) ||
       (NULL == CU_add_test(sharedSuite, "Compare: Less...............", test_compare_less)) ||
       (NULL == CU_add_test(sharedSuite, "Compare: Greater............", test_compare_more)) ||
       (NULL == CU_add_test(sharedSuite, "Select Pivot, Odd...........", test_select_pivot_odd)) ||
       (NULL == CU_add_test(sharedSuite, "Select Pivot, Even..........", test_select_pivot_even)) ||
       (NULL == CU_add_test(sharedSuite, "Swap Ints...................", test_swap_ints)) ||
       (NULL == CU_add_test(sharedSuite, "Power Function, Even........", test_power_even)) ||
       (NULL == CU_add_test(sharedSuite, "Power Function, Odd.........", test_power_odd)) ||
       (NULL == CU_add_test(sharedSuite, "Value Partitiony............", test_partition_by_val)) ||
       (NULL == CU_add_test(sharedSuite, "Array Union.................", test_array_union)) ||
       (NULL == CU_add_test(sharedSuite, "Subgroup Info...............", test_subgroup_info)) ||
       (NULL == CU_add_test(sharedSuite, "Compress Array..............", test_compress_array)) ||
       (NULL == CU_add_test(sharedSuite, "Index Partition: First......", test_partition_by_index_first)) ||
       (NULL == CU_add_test(sharedSuite, "Index Partition: Mid........", test_partition_by_index_mid)) ||
       (NULL == CU_add_test(sharedSuite, "Index Partition: Last.......", test_partition_by_index_last)) ||
       (NULL == CU_add_test(sharedSuite, "Index Partition: Small Val..", test_partition_by_index_smallest_val)) ||
       (NULL == CU_add_test(sharedSuite, "Index Partition: Large Val..", test_partition_by_index_largest_val)) ||
       (NULL == CU_add_test(sharedSuite, "Select Index: First.........", test_select_kth)) ||
       (NULL == CU_add_test(sharedSuite, "Median of Medians: First....", test_median_of_medians))
      )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
