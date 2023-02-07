#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//------------------------------
// MPI constants
//------------------------------
const int MASTER = 0; // taskid of first task
const int FROM_MASTER = 1; // message types
const int FROM_WORKER = 2;

// Choose whether to include debug features and logging.
// #define DEBUG

// Simple debug macro
#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

//------------------------------
// Math formulas
//------------------------------
double f(const double x);
double h_x(const double, const double, const int);

double map(double x, double in_min, double in_max, double out_min, double out_max); 

//------------------------------
// MPI helper functions
//------------------------------
void init_rand_seed(void); 
double estimate_g(const double, const double, const long long);
void collect_results(const double* const);
DBG(void test_rng(const double, const double);)


//==============================
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    // Define constants
    const double lower_bound = atof(argv[1]);
    const double upper_bound = atof(argv[2]);
    const long long N = atof(argv[3]);

    // Initialize different seeds to get better variance among workers
    init_rand_seed();

    // Test ranges just to be sure they work correctly
    DBG(test_rng(lower_bound, upper_bound);)

    // Run N estimations and send them to the master.
    double result = estimate_g(lower_bound, upper_bound, N);

    // The master will tally up the estimations, yielding the final result.
    collect_results(&result);

    MPI_Finalize();
    return 0;
}


//------------------------------
// Math formulas
//------------------------------
/**
 * The formula to be integrated.
 */
double f(double x) {
    return 8 * sqrt(2 * M_PI) / exp(pow(2 * x, 2));
}

/**
 * The monte-carlo integral.
 * Used by workers to obtain their results.
 * n: number of iterations.
 */
double h_x(const double lower_bound, const double upper_bound, const int n) {
    double sum = 0;

    for (int i = 0; i < n; i++) {
        double random_sample = map(rand(), 0, RAND_MAX, lower_bound, upper_bound);
        sum += f(random_sample);
    }

    return (upper_bound - lower_bound) / n * sum; // TODO should this only return sum, and the rest be calculated later in collect_results()?
}

/**
 * Linearly map an input (with range [in_min, in_max]) to [out_min, out_max].
 */
double map(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//------------------------------
// MPI helper functions
//------------------------------
/**
* Initialize a random seed for this worker.
* Use this worker's rank to seed the RNG.
*/
void init_rand_seed(void) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    srand(my_rank);

    DBG(printf("Seed initialized for rank %d.\n", my_rank));
}

DBG(
/**
* Print a few random numbers.
*/
void test_rng(const double lower_bound, const double upper_bound) {
    for(int i = 0; i < 100; i++) {
        // Get a random number and map it to the valid range.
        double random_num = map(rand(), 0, RAND_MAX, lower_bound, upper_bound);
        printf("%f, ", random_num);
        printf("\n");

        // Verify it's within the allowed range.
        // Honestly this tests `map` moreso than `rand`.
        if (random_num < lower_bound || random_num > upper_bound) {
            printf("ERROR: RNG is not mapping to [%f, %f] correctly.\n", lower_bound, upper_bound);
            exit(EXIT_FAILURE);
        }
    }
}
)

/**
 * Workers estimate their portion.
 */
double estimate_g(const double lower_bound, const double upper_bound, const long long N) {
    // Divide iterations evenly among all processes
    int num_nodes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_nodes);
    const int num_iterations = N / num_nodes;

    return h_x(lower_bound, upper_bound, num_iterations);
}


/**
* The workers send their results to the master.
* Master collects the results of each worker and sums 
* them together.
*/
void collect_results(const double* const result) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == MASTER) {
        // Collect data from workers
        int num_nodes;
        MPI_Comm_size(MPI_COMM_WORLD, &num_nodes);

        double sum = *result;

        for (int i = 1; i < num_nodes; i++) {
            double worker_result;
            MPI_Recv(&worker_result, 1, MPI_DOUBLE, i, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            sum += worker_result;
        }

        // To take the mean, divide by num_nodes
        sum /= num_nodes;

        // print stats
        printf("Approximation: %f\n", sum);
    }
    else {
        // Send data to master
        MPI_Send(result, 1, MPI_DOUBLE, MASTER, FROM_WORKER, MPI_COMM_WORLD);
    }
}

