/**
 * Reference implementation of quicksort with serial implementation as baseline.
 * This program will only serially calculate the quicksort of the input file.
 * If you don't know the number of words in your input: cat input.txt | wc, second number is word count.
 * See mylib.h/.c for functions not in this file.
 *
 * Use command: bsub -I -q COMP428 -n1 mpirun -srun ./demo/parallel <work> <mode>
 *
 * Arguments to serial:
 * work: The amount of numbers per process, total = work * world size.
 * mode: Flag that optionally makes master generate a new input.
 * 		-> Use "gen" to generate new input.
 * 		-> Use "read" to use existing input.txt.
 * TODO:
 *  -> Resolve outstanding scaling issue. Memory limited?
 *  -> Resolve CUnit issue on cirrus.
 */
/********************* Header Files ***********************/
/* C Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Project Headers */
#include "mpi.h"
#include "array_ops.h"
#include "file_ops.h"

/******************* Constants/Macros *********************/
#define BUF_SIZE 			1000000
#define GATHER_SCALE 		1.2
#define QDEBUG 				1 // Enable this line for tracing code.
#define LOG_SIZE			100
/* Maximum dimension of the hypercube */
#define MAX_DIM 			3
/* A tag for use in the send and recv */
#define PIVOT_TAG			0
#define EXCHANGE_TAG 		1

/******************* Type Definitions *********************/


/**************** Static Data Definitions *****************/
/* Log file per process, buf used for anything not an array trace. */
static FILE *log;
static char log_buf[LOG_SIZE];

/****************** Static Functions **********************/


/**************** Global Data Definitions *****************/


/****************** Global Functions **********************/

/*
 * Simple wrapper, acts as a multicast but only sends to members of a given subgroup.
 * Only ever called by root of a group, send to all ids above sender below group size.
 */
void send_pivot(int pivot, const subgroup_info_t * const info) {
    MPI_Request mpi_request;

    for (int i = 1; i < info->group_size; ++i)
        MPI_Isend(&pivot, 1, MPI_INT, info->world_id+i, PIVOT_TAG, MPI_COMM_WORLD, &mpi_request);
}

/*
 * Implementation of the hyper quicksort for any given dimension. Topology is assumed to be entirely
 * in MPI_COMM_WORLD. Details follow traditional hypercube algorithm seen on page 422 of Parallel Computing (Gupta).
 * At the end, each processor with local_size elements in local will be ready to locally sort.
 */
void hyper_quicksort(const int dimension, const int id, int *local[], int *local_size,
        int recv[], const int recv_size) {
    MPI_Status mpi_status;
    MPI_Request mpi_request;
    subgroup_info_t info = {0, 0, 0, 0, id};
    int pivot = 0, lt_size = 0, gt_size = 0, received = 0;

    /* Iterate for all dimensions of cube. */
    for (int d = dimension-1; d >= 0; --d) {
        /* Determine the group and member number of id, and its partner. */
        lib_subgroup_info(d+1, &info);

        /* Select and broadcast pivot only to subgroup. */
        if (info.member_num == 0) {
            pivot = lib_select_pivot(*local, *local_size);
            snprintf(log_buf, LOG_SIZE, "ROUND: %d, GROUP: %d, pivot is: %d.\n", dimension-d, info.group_num, pivot);
            lib_log(log, "PIVOT", log_buf);
            send_pivot(pivot, &info);
        } else {
            MPI_Recv(&pivot, 1, MPI_INT, MPI_ANY_SOURCE, PIVOT_TAG, MPI_COMM_WORLD, &mpi_status);
        }

        /* Partition the array. */
        lib_partition_by_pivot_val(pivot, *local, *local_size, &lt_size, &gt_size);

        /* Barrier here ensures all outstanding pivot recvs complete. */
#ifdef QDEBUG
        lib_trace_array(log, "PARTITIONED", *local, *local_size);
#endif

        /* Determine position in the cube. If below is true, I am in upper part of this dimension. */
        if (id & (1<<d)) {
            MPI_Isend(*local, lt_size, MPI_INT, info.partner, EXCHANGE_TAG, MPI_COMM_WORLD, &mpi_request);
            MPI_Recv(recv, recv_size, MPI_INT, info.partner, EXCHANGE_TAG, MPI_COMM_WORLD, &mpi_status);
            /* We have sent lower portion, move elements greater down. Update local_size.*/
            memmove(*local, *local+lt_size, gt_size*sizeof(int));
            *local_size = gt_size;
        } else {
            MPI_Isend(*local+lt_size, gt_size, MPI_INT, info.partner, EXCHANGE_TAG, MPI_COMM_WORLD, &mpi_request);
            MPI_Recv(recv, recv_size, MPI_INT, info.partner, EXCHANGE_TAG, MPI_COMM_WORLD, &mpi_status);
            /* We have sent upper portion of array, merely update size and ignore older elements. */
            *local_size = lt_size;
        }

        /* Get the received count and call array union function to merge into local. */
        MPI_Get_count(&mpi_status, MPI_INT, &received);
        lib_array_union(local, local_size, recv, received);

        /* Ensure all partner exchanges complete before proceeding to the next round. */
#ifdef QDEBUG
        lib_trace_array(log, "RECV", recv, received);
        lib_trace_array(log, "UNION", *local, *local_size);
#endif
    }
}

/*
 * Main execution body.
 */
int main(int argc, char **argv) {
    int id = 0, world = 0, num_proc = 0, root_size = 0, recv_size = 0, local_size = 0;
    int *root = NULL, *recv = NULL, *local = NULL;
    char file[FILE_SIZE];
    double start = 0.0;

    /* Standard init for MPI, start timer after init. Get rank and size too. */
    MPI_Init(&argc, &argv);
    start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    /* Open log file, overwrite on each open. */
    snprintf(file, FILE_SIZE, LOG_FORMAT, id);
    if ((log = fopen(file, "w")) == NULL)
        lib_error("MAIN: Could not open log file.");

    /* Protection from invalid use. */
    if (argc < 3)
        lib_error("MAIN: Bad usage, see top of respective c file.");

    /* Get the work amount from command for each process. */
    num_proc = atoi(*++argv);
    root_size = num_proc * world;

    /* Root only work, ensure good usage and proper input. */
    if (id == ROOT) {
        /* Allocate the whole array on the heap, large amount of memory likely wouldn't fit on stack. */
        root = (int *)malloc(root_size * sizeof(int));
        if (root == NULL)
            lib_error("MAIN: Can't allocate root_vals array on heap.");

        /* If requested, generate new input file. */
        if (strcmp(*++argv, GENERATE_FLAG) == 0) {
            lib_generate_numbers(root, root_size);
            lib_write_file(INPUT, root, root_size);
        }

        /* Read back input from file into array on heap. */
        lib_read_file(INPUT, root, root_size);
    }

    /*
     * Allocate a recv buf of root_size (though not likely needed) and a local size of n/p.
     * The recv buf accounts for the unlikely but possible lop sideded partitioning.
     */
    recv_size = num_proc * GATHER_SCALE;
    local_size = num_proc;
    recv = (int *)malloc(recv_size * sizeof(int));
    if (recv == NULL)
        lib_error("MAIN: Can't allocate recv array on heap.");

    local = (int *)malloc(local_size * sizeof(int));
    if (local == NULL)
        lib_error("MAIN: Can't allocate local array on heap.");

    /* Scatter across the processes and then do hyper quicksort algorithm. */
    MPI_Scatter(root, num_proc, MPI_INT, local, local_size, MPI_INT, ROOT, MPI_COMM_WORLD);

#ifdef QDEBUG
    lib_trace_array(log, "SCATTER", local, local_size);
#endif

    /* Rearrange the cube so that each processor has data strictly less than one with higher number.*/
    hyper_quicksort(MAX_DIM, id, &local, &local_size, recv, recv_size);

#ifdef QDEBUG
    lib_trace_array(log, "HYPER", local, local_size);
#endif

    /*
     * Reallocated root to be rescaled, mpi_gather doesn't know how many per process anymore.
     * Set values to -1.
     */
    if (id == ROOT) {
        free(root);
        root_size *= GATHER_SCALE;
        root = (int *)malloc(root_size * sizeof(int));
        if (root == NULL)
            lib_error("MAIN: Can't allocate root array on heap.");
        memset(root, -1, root_size * sizeof(int));
    }

    /* Quicksort local array and then send back to root. */
    qsort(local, local_size, sizeof(int), lib_compare);
    MPI_Gather(local, local_size, MPI_INT, root, GATHER_SCALE*num_proc, MPI_INT, ROOT, MPI_COMM_WORLD);

	/* Last step, root has to compress array due to uneven nature after gather. Then write to file. */
    if (id == ROOT) {
        lib_compress_array(world, root, root_size);

#ifdef QDEBUG
        lib_trace_array(log, "GATHER", root, root_size/GATHER_SCALE);
#endif

        lib_write_file(OUTPUT, root, root_size/GATHER_SCALE);
        printf("Time elapsed from MPI_Init to MPI_Finalize is %.10f seconds.\n", MPI_Wtime() - start);
        free(root);
    }

    free(recv);
    /* May have been entirely deallocated if has no more at process. */
    if (local != NULL)
        free(local);

    fclose(log);

    MPI_Finalize();

    return 0;
}
