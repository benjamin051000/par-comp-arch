#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

#define DEBUG

//////////////////////////////
// Useful debugging macros
//////////////////////////////
#ifdef DEBUG
#define DBG(x) x
#define DP(x, ...) (printf("[%s:%d] ", __FILE__, __LINE__), printf(x, __VA_ARGS__))
#else
#define DBG(x)
#define DP(x)
#endif

#ifdef RELEASE_
#define RELEASE(x) x
#else
#define RELEASE(x) 
#endif


//////////////////////////////
// Performance Metrics
//////////////////////////////
double tdiff(struct timespec a, struct timespec b) {
    // Find the time difference.
    return ( b.tv_sec - a.tv_sec ) + ( b.tv_nsec - a.tv_nsec ) / 1E9;
}

struct timespec now() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t;
}


//////////////////////////////
// Helper functions
//////////////////////////////
// Prints the name of the variable alongside the matrix itself!
#define print_matrix(M, N, A) (__print_matrix(M, N, #A, A))
void __print_matrix(const int M, const int N, const char* const name, const int (* const matrix)[N]) {
    printf("=============== Matrix %s ===============\n", name);
    for(int r = 0; r < M; r++) {
        for(int c = 0; c < N; c++) {
            printf("%3d ", matrix[r][c]);
        }
        printf("\n");
    }
    printf("=============== End of %s ===============\n", name);
}


void init_matrix(const int rows, const int cols, int A[rows][cols], char plus_or_minus) {
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < cols; c++) {
            int temp;

            if(plus_or_minus == '+') {
                temp = r + c;
            }
        else {
                temp = r - c;
            }

            A[r][c] = temp;
        }
    }
}


//////////////////////////////
// Main
//////////////////////////////
int main(int argc, char* argv[]) {
    // 3 args + 1 for the first arg, which is always the process name
    DP("argc = %d\n", argc);

    if (argc != 3 + 1) {
        printf("Usage: ./matmult <rows in A> <cols in A/rows in B> <cols in B>");
        exit(EXIT_FAILURE);
    }

    const int NRA = atof(argv[1]);
    const int NCA_RB = atof(argv[2]);
    const int NCB = atof(argv[3]);

    // Define arrays
    int A[NRA][NCA_RB];
    init_matrix(NRA, NCA_RB, A, '+');
    DBG(print_matrix(NRA, NCA_RB, (const int (*)[])A);)

    int B[NCA_RB][NCB];
    init_matrix(NCA_RB, NCB, B, '-');
    DBG(print_matrix(NCA_RB, NCB, (const int (*)[])B);)

    int C[NRA][NCB];

    DP("Total # threads: %d\n", omp_get_num_threads());

    const struct timespec start = now();

    int tid;
    #pragma omp parallel shared(A, B, C) private(tid)
    {
        DBG(tid = omp_get_thread_num();)
        DP("Hello from thread %d\n", tid); 

        #pragma omp for // defaults to kind=static, chunk=N/num_threads
        for (int r = 0; r < NRA; r++) {
            for (int c = 0; c < NCB; c++) {
                // Not sure what this inner loop is for 
                C[r][c] = 0;
                for (int k = 0; k < NCA_RB; k++) {
                    C[r][c] += A[r][k] * B[k][c];
                }
            } // end of for col
        } // end of for row
    }

    const struct timespec stop = now();
    
    print_matrix(NRA, NCB, (const int (*)[])C);

    const double elapsed = tdiff(start, stop);
    printf("Elapsed time: %f sec\n", elapsed);

    printf("Goodbye.\n");
    return 0;
}

