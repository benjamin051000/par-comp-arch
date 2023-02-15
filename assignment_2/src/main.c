#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

//////////////////////////////
// Macros & Constants
//////////////////////////////
#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

const int MASTER = 0;
const int FROM_MASTER = 1;
const int FROM_WORKER = 0;

//////////////////////////////
// Helper functions
//////////////////////////////
int rand_range(const int min, const int max);

void print_matrix(const int N, const int (* const matrix)[N]);

//////////////////////////////
// Function declarations
//////////////////////////////
void* initialize_data(const int N);

int* distribute_data(const int N, int (*matrix)[N]);

int* mask_operation(int* recv_buf, int N);

void collect_results(int* updated_buf, int N, int* Ap);

 
//////////////////////////////
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    const int N = atof(argv[1]);

    // Initialize the matrix
    int (*matrix)[N] = initialize_data(N);

    int* temp1 = distribute_data(N, matrix);
    int* temp2 = mask_operation(temp1, N);
    collect_results(temp2, N, NULL);

    MPI_Finalize();
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

void print_matrix(const int N, const int (* const matrix)[N]) {
    printf("=============== Matrix ===============\n");
    for(int r = 0; r < N; r++) {
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
    int (*matrix)[N] = malloc(sizeof(int[N][N]));

    if (matrix == NULL) {
        printf("ERROR: Couldn't allocate matrix.\n");
        exit(EXIT_FAILURE);
    }

    // seed the RNG
    srand(1);

    // Populate the matrix with random numbers
    for(int r = 0; r < N; r++) {
        for(int c = 0; c < N; c++) {
            matrix[r][c] = rand_range(0, 255);
        }
    }

    print_matrix(N, matrix);

    return matrix;
}

/**
*
*/
int* distribute_data(const int N, int (*matrix)[N]) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank != MASTER) {
        return NULL;
    }

    print_matrix(N, matrix);

    return NULL;
}

/**
*
*/
int* mask_operation(int* recv_buf, int N) {

}

/**
*
*/
void collect_results(int* updated_buf, int N, int* Ap) {

}

