#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define DEBUG

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

#ifdef RELEASE_
#define RELEASE(x) x
#else
#define RELEASE(x) 
#endif

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

int main(int argc, char* argv[]) {
    // 3 args + 1 for the first arg, which is always the process name
    DBG(printf("argc = %d\n", argc);)

    if (argc != 3 + 1) {
        printf("Usage: ./matmult <> <> <> <>");
        exit(EXIT_FAILURE);
    }

    const int NRA = atof(argv[1]);
    const int NCA_RB = atof(argv[2]);
    const int NCB = atof(argv[3]);
    
    // Define arrays
    int A[NRA][NCA_RB];
    init_matrix(NRA, NCA_RB, A, '+');
    print_matrix(NRA, NCA_RB, (const int (*)[])A);

    int B[NCA_RB][NCB];
    init_matrix(NRA, NCA_RB, A, '-');
    print_matrix(NRA, NCA_RB, (const int (*)[])B);

    int C[NRA][NCB];

    DBG(printf("Total # threads: %d\n", omp_get_num_threads());)

    int tid;
    #pragma omp parallel shared(A, B, C) private(tid)
    {
        DBG(
            tid = omp_get_thread_num();
            printf("Hello from thread %d\n", tid);
        )

        #pragma omp for // defaults to kind=static, chunk=N/num_threads
        for (int r = 0; r < NRA; r++) {
            for (int c = 0; c < NCB; c++) {
                // Not sure what this inner loop is for 
                for (int k = 0; k < NCA_RB; k++) {
                    C[r][c] = A[r][k] * B[k][c];
                }
            } // end of for col
        } // end of for row
    }

    
    print_matrix(NRA, NCB, (const int (*)[])C);

    printf("Goodbye.\n");
    return 0;
}

