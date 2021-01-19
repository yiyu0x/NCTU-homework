#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---
    // TODO: init MPI
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Get_processor_name(processor_name, &name_len);
    //
    int NUMBER_OF_TOSSES_IN_JOB = 0;
    const int REMINDER_OF_TOSSES = tosses % world_size;
    NUMBER_OF_TOSSES_IN_JOB = tosses / world_size;
    //
    if (world_rank > 0)
    {   
        srand(time(0)*world_rank);
        int number_in_circle_thread = 0;
        unsigned int rand_state = rand();
        for (long long int toss = 0; toss < NUMBER_OF_TOSSES_IN_JOB; toss++) {
            double x = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
            double y = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
            double distance_squared = x * x + y * y;
            if (distance_squared <= 1) {
                number_in_circle_thread++;
            }
        }
        // TODO: handle workers
        /*
        MPI_Send(
            void* data,
            int count,
            MPI_Datatype datatype,
            int destination,
            int tag,
            MPI_Comm communicator
        )
        */
        // printf("%d -> number_in_circle_thread: %d\n", world_rank, number_in_circle_thread);
        MPI_Send(&number_in_circle_thread, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    else if (world_rank == 0)
    {
        // TODO: master
        srand(time(0)*world_rank);
        int number_in_circle_thread = 0;
        unsigned int rand_state = rand();
        for (long long int toss = 0; toss < NUMBER_OF_TOSSES_IN_JOB + REMINDER_OF_TOSSES; toss++) {
            double x = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
            double y = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
            double distance_squared = x * x + y * y;
            if (distance_squared <= 1) {
                number_in_circle_thread++;
            }
        }
        // printf("%d -> number_in_circle_thread: %d\n", world_rank, number_in_circle_thread);
        MPI_Send(&number_in_circle_thread, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (world_rank == 0)
    {
        // TODO: process PI result
        /*
        MPI_Recv(
            void* data,
            int count,
            MPI_Datatype datatype,
            int source,
            int tag,
            MPI_Comm communicator,
            MPI_Status* status
        )
        */
        int counter = world_size;
        long long int sum = 0;
        while(counter--) {
            int data;
            MPI_Recv(&data, 1, MPI_INT, counter, 0, MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
            sum += data;
        }
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
