#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

int toss(int times, int world_rank) {
    srand(time(0)*world_rank);
    int number_in_circle_thread = 0;
    unsigned int rand_state = rand();
    for (long long int toss = 0; toss < times; toss++) {
        double x = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
        double y = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
        double distance_squared = x * x + y * y;
        if (distance_squared <= 1) {
            number_in_circle_thread++;
        }
    }
    return number_in_circle_thread;
}

int main(int argc, char **argv)
{
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---

    // TODO: MPI init
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    const int NUMBER_OF_TOSSES_IN_JOB = tosses / world_size;
    // TODO: use MPI_Reduce
    long long int sum = 0;
    int count = toss(NUMBER_OF_TOSSES_IN_JOB, world_rank);
    MPI_Reduce(&count, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        // TODO: PI result
        pi_result = sum * 4 / (double)tosses;
        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
