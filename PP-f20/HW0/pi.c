#include <omp.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h> 
#include <time.h> 

#define NUMBER_OF_TOSSES ULLONG_MAX

int main() {
    srand(time(0));
    double number_in_circle = 0;

    #pragma omp parallel for
    for (long long int toss = 0; toss < NUMBER_OF_TOSSES; toss++) {
        double x = (double)rand() / (double)((unsigned)RAND_MAX + 1);
        double y = (double)rand() / (double)((unsigned)RAND_MAX + 1);
        double distance_squared = x * x + y * y;
        if (distance_squared <= 1) {
            #pragma omp critical
            number_in_circle++;
        }
    }
    double pi_estimate = 4 * number_in_circle /((double)NUMBER_OF_TOSSES);
    printf("%.7f...", pi_estimate);
}