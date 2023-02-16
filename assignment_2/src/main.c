#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

//////////////////////////////
// Macros & Constants
//////////////////////////////
#define DEBUG

#ifdef DEBUG
#define DBG(x) x
#define RELEASE(x)
#else
#define DBG(x)
#define RELEASE(x) x
#endif

const int MASTER = 0;
const int FROM_MASTER = 1;
const int FROM_WORKER = 0;

//////////////////////////////
// Helper functions
//////////////////////////////
int rand_range(const int min, const int max);

void print_matrix(const int M, const int N, const int (* matrix)[M]);

//////////////////////////////
// Function declarations
//////////////////////////////
void* initialize_data(const int N);

void* distribute_data(const int N, int (*matrix)[N]);

void* mask_operation(const int N, int (*worker_submatrix)[N]);

// void collect_results(const int N, int (*updated_buf)[N], int* Ap);

 
//////////////////////////////
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    const int N = atof(argv[1]);

    // Initialize the matrix
    int (* const matrix)[N] = initialize_data(N);

    int (*worker_submatrix)[] = distribute_data(N, matrix);
    int (*processed_submatrix)[] = mask_operation(N, worker_submatrix);
    // collect_results(N, processed_submatrix, NULL);

    MPI_Finalize();
    free(matrix);
    // TODO BUG we are leaking the worker submatrices!
    return 0;
}

/**
 * Generate a (pseudo)random number within a range.
 * Returns a random number in the range [min, max]. (Inclusive)
 * Apparently using FP math here is less biased than modulo math.
 * See https://stackoverflow.com/questions/9571738/picking-random-number-between-two-points-in-c
 */
int rand_range(const int min, const int max) {
    const int diff = max - min;
    return (int) (((double)(diff + 1) / RAND_MAX) * rand() + min);
}

/**
 * Print an MxN matrix.
 * M: rows
 * N: cols
 */
void print_matrix(const int M, const int N, const int (* const matrix)[N]) {
    printf("=============== Matrix ===============\n");
    for(int r = 0; r < M; r++) {
        for(int c = 0; c < N; c++) {
            printf("%3d ", matrix[r][c]);
        }
        printf("\n");
    }
    printf("======================================\n");
}

/**
* Initialize and return data for
* an NxN matrix A.
*/
void* initialize_data(const int N) {
    // Only the 1st rank should initialize the matrix.
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank != MASTER) {
        return NULL;
    }

    // Create the array
    // Using epic c99 array pointer syntax :D
    // see https://stackoverflow.com/questions/32050256/function-to-dynamically-allocate-matrix
    //
    // NOTE: The type declaration needs cols in the square brackets. So it's `int (*)[num_cols]`.
    // sizeof doesn't matter because it just makes a block

    const int M = N; // For consistency with MxN matrices.
    int (*matrix)[N] = malloc(sizeof(int[M][N]));

    if (matrix == NULL) {
        printf("ERROR: Couldn't allocate matrix.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // seed the RNG
    srand(1);

    // Populate the matrix with random numbers
    for(int r = 0; r < M; r++) {
        for(int c = 0; c < N; c++) {
            DBG(
                // Makes it easier to tell matrix is populated correctly
                matrix[r][c] = r * N + c;
            )
            RELEASE(
                matrix[r][c] = rand_range(0, 255);
            )
        }
    }

    DBG(print_matrix(M, N, (const int (*)[])matrix);)

    return matrix;
}




/**
*
*/
void* distribute_data(const int N, int (*matrix)[N]) {
    // This worker's rank
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    // Total number of ranks.
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    DBG(printf("rank %d of %d checking in\n", my_rank, num_ranks);)

    const int M = N; // stay consistent with MxN matrix 
    const int num_elems = M * N;

    const int num_rows_per_worker = M / num_ranks;
    DBG(printf("num_rows_per_worker=%d\n", num_rows_per_worker);)

    // const int num_elems_per_worker = num_rows_per_worker * N;

    // Number of elements to send to each processor
    int* const sendcounts = malloc(num_ranks * sizeof *sendcounts);

    const int num_elems_per_worker = num_rows_per_worker * N; // N is num elems per row
    printf("num_elems_per_worker=%d\n", num_elems_per_worker);

    for(int i = 0; i < num_ranks; i++) {
        sendcounts[i] = num_elems_per_worker;
    }

    // Where should each chunk begin?
    // const int displs[] = {0, num_elems/2};
    DBG(printf("displacements: [");)
    int* const displs = malloc(num_ranks * sizeof *displs);
    for(int i = 0; i < num_ranks; i++) {
        displs[i] = i * num_elems_per_worker;
        DBG(printf("%d ", displs[i]);)
    }
    DBG(printf("]\n");)

    const int recvbuf_size = sizeof(int) * N * num_rows_per_worker;
    printf("recvbuf_size / sizeof(int)=%lu\n", recvbuf_size / sizeof(int));


    int (*recvbuf)[] = malloc(recvbuf_size);
    if(recvbuf == NULL) {
        printf("ERROR: Couldn't allocate recvbuf.\n");
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    DBG(printf("rank %d got here\n", my_rank);)
    MPI_Scatterv(matrix, sendcounts, displs, MPI_INT, recvbuf, num_elems_per_worker, MPI_INT, MASTER, MPI_COMM_WORLD);

    DBG(printf("rank %d got here\n", my_rank);)

    return recvbuf;
}

/**
* Perform the weighted-averaging filter.
* For each element in the matrix, compute
* the average of it and its 8 surrounding
* neighbors.
*
* Note: Processing of edges is not required.
*/
void* mask_operation(int N, int (*worker_submatrix)[N]) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    const int num_rows = N / num_ranks;
    printf("Rank %d:\n", my_rank);
    print_matrix(num_rows, N, (const int (*)[])worker_submatrix);
    return NULL;
}

/**
*
*/
// void collect_results(int N, int (*updated_buf)[N], int* Ap) {
//
// }
//
