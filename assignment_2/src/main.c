#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
// Structures
//////////////////////////////
struct recvbuf_and_size {
    void* recvbuf;
    int size;
};

struct sendcounts_displacements {
    int* sendcounts;
    int* displacements;
};

//////////////////////////////
// Function declarations
//////////////////////////////
void* initialize_data(const int N);

struct recvbuf_and_size distribute_data(const int N, int (*matrix)[N]);

void* mask_operation(const int N, const int worker_submatrix_size, int (*worker_submatrix)[N]);

// void collect_results(const int N, int (*updated_buf)[N], int* Ap);

 
//////////////////////////////
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    const int N = atof(argv[1]);

    // Initialize the matrix
    int (* const matrix)[N] = initialize_data(N);

    // int (*worker_submatrix)[] = distribute_data(N, matrix);
    struct recvbuf_and_size r_s = distribute_data(N, matrix);
    int (* const worker_submatrix)[] = r_s.recvbuf;
    const int worker_submatrix_size = r_s.size;

    int (*processed_submatrix)[] = mask_operation(N, worker_submatrix_size, worker_submatrix);
    // collect_results(N, processed_submatrix, NULL);

    MPI_Finalize();
    free(worker_submatrix);
    free(matrix);

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


// TODO extract a "divide evenly" function to make this easier to understand
struct sendcounts_displacements generate_sendcounds_and_displacements(
    const int num_ranks, 
    const int M, // num rows
    const int N, // AKA num elements per row, or num cols
    const int num_rows_per_worker
) {
    const int sendcounts_size = num_ranks * sizeof(int);
    int* const sendcounts = malloc(sendcounts_size);
    int* const displacements = malloc(num_ranks * sizeof *displacements);

    // Keeps track of odd-numbered matrices, to ensure each row
    // is placed in a worker's group.
    int remainder = M % num_ranks;

    // Total number of data assigned to workers
    int sum = 0;

    // TODO Add additional rows for offsets!
    for(int rank = 0; rank < num_ranks; rank++) {
        // The buffer technically must be number of rows * elements per row
        sendcounts[rank] = num_rows_per_worker * N;

        if (remainder) {
            // Give one extra row to this worker.
            sendcounts[rank] += N;
            // One less to worry about 
            remainder--;
        }

        displacements[rank] = sum;

        // int my_rank; MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        // Add one additional row above and one below for neighboring purposes.
        // Master rank only: Don't add another row above because there isn't one.
        // You already have the first row, just don't calculate it.
        if (rank != MASTER) {
            // For other ranks, add a row above.
            sendcounts[rank] += N;

            // Sum shouldn't include the extra rows used for neighboring tiles.
            sum -= N;

            // Displacements should be shifted down by one row to accomodate for this.
            displacements[rank] -= N;
        }

        // Last rank only: Don't add another row below because there isn't one.
        // You already have the last row, just don't calculate it.
        if (rank != num_ranks - 1) {
            // For other ranks, add a row below.
            sendcounts[rank] += N;
            // No need to shift displacements since this was added to the end.

            // Sum shouldn't include the extra rows used for neighboring tiles.
            sum -= N;
        }

        // Sum shouldn't include the extra rows used for neighboring tiles.
        // This has already been accounted for!
        sum += sendcounts[rank];
    }

    DBG(
        printf("sendcounts (# rows): [ ");
        for(int i = 0; i < num_ranks; i++) {
            printf("%d ", sendcounts[i] / N);
        }
        printf("]\n");
    )

    DBG(
        printf("displacements: [ ");
        for(int i = 0; i < num_ranks; i++) {
            printf("%d ", displacements[i]);
        }
        printf("]\n");
    )
    
    return (struct sendcounts_displacements) {
        sendcounts,
        displacements
    };
}


struct recvbuf_and_size distribute_data(const int N, int (*matrix)[N]) {
    // This worker's rank
    int my_rank; MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    // Total number of ranks.
    int num_ranks; MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    DBG(printf("rank %d of %d checking in\n", my_rank, num_ranks);)

    const int M = N; // stay consistent with MxN matrix 
    const int num_elems = M * N;

    // Number of elements to send to each processor, assuming it divides evenly.
    const int num_elems_per_worker = num_elems / num_ranks; // N is num elems per row
    // Number of rows to send to each processor, assuming they can be divided evenly.
    const int num_rows_per_worker = M / num_ranks;
    DBG(printf("num_elems_per_worker=%d\n", num_elems_per_worker);)

    struct sendcounts_displacements s_d = generate_sendcounds_and_displacements(num_ranks, M, N, num_rows_per_worker);
    int* const sendcounts = s_d.sendcounts;
    int* const displacements = s_d.displacements;

    // Each buffer size varies based off what was allotted for this rank.
    const int recvbuf_size = sendcounts[my_rank];
    DBG(printf("rank %d recvbuf_size = %d\n", my_rank, recvbuf_size);)

    int (* const recvbuf)[N] = malloc(recvbuf_size * sizeof **recvbuf);
    if(recvbuf == NULL) {
        printf("ERROR: Couldn't allocate recvbuf.\n");
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    MPI_Scatterv(
        matrix, // ref to original data
        sendcounts,
        displacements,
        MPI_INT,
        recvbuf, // ref to receive buf
        recvbuf_size, // number of elements in receive buffer
        MPI_INT,
        MASTER, // who is doing the sending
        MPI_COMM_WORLD
    );

    // Done with these (?)
    free(sendcounts);
    free(displacements);

    return (struct recvbuf_and_size) {
        recvbuf,
        recvbuf_size
    };
}

/**
* Perform the weighted-averaging filter.
* For each element in the matrix, compute
* the average of it and its 8 surrounding
* neighbors.
*
* Note: Processing of top, bottom, left, and right edges is not required.
*/
void* mask_operation(int N, const int worker_submatrix_size, int (*worker_submatrix)[N]) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    // Get num rows, but round up to nearest row.
    const int M = (worker_submatrix_size + (N - 1)) / N;

    printf("Rank %d:\n", my_rank);
    printf("worker submatrix size: %d\n", worker_submatrix_size);
    printf("N: %d\n", N);
    print_matrix(M, N, (const int (*)[])worker_submatrix);

    

    // Get the coords of which results need to be calculated.
    // The first and last rows are only used for neighbor calculations.
    const int startx = 1;
    const int endx = M - 1;
    const int resulting_num_rows = endx - startx;
    // First and last columns skipped for same reason.
    const int starty = 1;
    const int endy = N - 1; 
    const int resulting_num_cols = N; // Preserve same number of cols as before

    // Create the output subatrix. Make it 2 rows smaller than 
    // the incoming one since we don't need those two rows.
    int (*results)[resulting_num_cols] = calloc(resulting_num_rows * resulting_num_cols, sizeof(int));
    if (results == NULL) {
        printf("ERROR: Couldn't allocate results.\n");
        MPI_Abort(MPI_COMM_WORLD, 4);
    }

    for(int x = startx; x < endx; x++) {
        for(int y = starty; y < endy; y++) {
            const int data = worker_submatrix[x][y];
            // DBG(printf("[rank %d] Working on submat[%d][%d] (which is %d)...\n", my_rank, x, y, data);)

            // Add the 3x3 square of neighbors (hence "nx" "ny") together
            int sum = 0;
            for(int nx = x - 1; nx <= x + 1; nx++) {
                for(int ny = y - 1; ny <= y + 1; ny++) {
                    sum += data;
                }
            }

            const int average = sum / 10;

            // Shift one row up, but maintain the column relative to the original.
            results[x-1][y] = average; // Save to results

            // DBG(printf("[rank %d] Average for tile: %d\n", my_rank, results[x-1][y-1]);)

        } // end of for c
    } // end of for r

    DBG(printf("[rank %d] resulting matrix:\n", my_rank);)
    DBG(print_matrix(resulting_num_rows, resulting_num_cols, (const int (*)[])results);)

    return results;
}

/**
*
*/
// void collect_results(int N, int (*updated_buf)[N], int* Ap) {
//
// }
//
