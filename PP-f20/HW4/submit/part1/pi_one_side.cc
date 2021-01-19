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

int fnz (int *schedule, int *oldschedule, int size, long long int *count) {
    int diff = 0;
    for (int i = 0; i < size; i++)
       diff |= (schedule[i] != oldschedule[i]);

    if (diff) {
       int res = 0;
       for (int i = 0; i < size; i++) {    
            if(schedule[i] != oldschedule[i]){
                *count += schedule[i];
            }
            if (schedule[i]) {
                res++;
            }
            oldschedule[i] = schedule[i];
       }
       return (res == size-1);
    }
    return 0;
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

    MPI_Win win;

    // TODO: MPI init
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    const int NUMBER_OF_TOSSES_IN_JOB = tosses / world_size;
    long long int count = toss(NUMBER_OF_TOSSES_IN_JOB, world_rank);
    
    if (world_rank == 0) {
        // Master
        int *oldschedule = (int*)malloc(world_size * sizeof(int));
        int *schedule;
        MPI_Alloc_mem(world_size * sizeof(int), MPI_INFO_NULL, &schedule);
        for (int i = 0; i < world_size; i++) {
            schedule[i] = 0;
            oldschedule[i] = 0;
        }

        MPI_Win_create(schedule, world_size * sizeof(int), sizeof(int), MPI_INFO_NULL,
            MPI_COMM_WORLD, &win);

        int ready = 0;
        while (!ready) {
            MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, win);
            ready = fnz(schedule, oldschedule, world_size, &count);
            MPI_Win_unlock(0, win);
        }

        free(oldschedule);
        MPI_Free_mem(schedule);
        
    } else {
        // Workers
        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
        MPI_Put(&count, 1, MPI_INT, 0, world_rank, 1, MPI_INT, win);
        MPI_Win_unlock(0, win);
    }

    MPI_Win_free(&win);

    if (world_rank == 0) {
        // TODO: handle PI result
        pi_result = count * 4 / (double)tosses;
        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }
    
    MPI_Finalize();
    return 0;
}