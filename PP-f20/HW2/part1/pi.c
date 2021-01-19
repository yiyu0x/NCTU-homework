#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct {
   int number_of_tosses;
   int* number_in_circle;
} Args;

void* job(void* args) {
    Args *data = (Args *)args;
    const long long int NUMBER_OF_TOSSES = data->number_of_tosses;
    int* number_in_circle = data->number_in_circle;
    long number_in_circle_thread = 0;
    unsigned int rand_state = rand();
    for (long long int toss = 0; toss < NUMBER_OF_TOSSES; toss++) {
        double x = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
        double y = (double)rand_r(&rand_state) / (double)((unsigned)RAND_MAX + 1);
        double distance_squared = x * x + y * y;
        if (distance_squared <= 1) {
            number_in_circle_thread++;
        }
    }
    pthread_mutex_lock(&mutex);
    *number_in_circle += number_in_circle_thread;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    srand(time(0));
    const int CPU_CORES = atoi(argv[1]);
    const long long int NUMBER_OF_TOSSES = atol(argv[2]);
    int NUMBER_OF_TOSSES_IN_JOB = 0;
    int NUMBER_OF_TOSSES_IN_JOB_REMAINDER = 0;
    const int HAS_REMINDER = NUMBER_OF_TOSSES % CPU_CORES;
    if (HAS_REMINDER) { // Has reminder
        NUMBER_OF_TOSSES_IN_JOB = NUMBER_OF_TOSSES / CPU_CORES;
        NUMBER_OF_TOSSES_IN_JOB_REMAINDER = NUMBER_OF_TOSSES_IN_JOB + (NUMBER_OF_TOSSES % (CPU_CORES));
    } else {
        NUMBER_OF_TOSSES_IN_JOB = NUMBER_OF_TOSSES / CPU_CORES;
    }
    
    int* number_in_circle = malloc(sizeof(*number_in_circle));
    *number_in_circle = 0;

    Args args_job;
    Args args_job_reminder;

    args_job.number_of_tosses = NUMBER_OF_TOSSES_IN_JOB;
    args_job.number_in_circle = number_in_circle;
    args_job_reminder.number_of_tosses = NUMBER_OF_TOSSES_IN_JOB_REMAINDER;
    args_job_reminder.number_in_circle = number_in_circle;

    pthread_t threads[CPU_CORES];

    for (int i = 0; i < CPU_CORES; i++) {
        if (HAS_REMINDER && i == CPU_CORES - 1) 
            pthread_create(&threads[i], NULL, job, (void*) &args_job_reminder);
        else 
            pthread_create(&threads[i], NULL, job, (void*) &args_job);
    }

    for (int i = 0; i < CPU_CORES; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);

    double pi_estimate = 4 * (*number_in_circle) / ((double)NUMBER_OF_TOSSES);
    printf("%.7f...\n", pi_estimate);
    
}