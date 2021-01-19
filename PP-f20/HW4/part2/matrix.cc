#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

void construct_matrices(int *n, int *m, int *l, int **a_mat, int **b_mat) {
    // MPI Init
    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    if (world_rank == 0) {
        scanf("%d %d %d", n, m, l);
    }

    MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(l, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // Sync n, m, l to other processes.
    *a_mat = (int*)malloc(sizeof(int) * (*n) * (*m));
    *b_mat = (int*)malloc(sizeof(int) * (*m) * (*l));

    if (world_rank == 0) {
        for(int i = 0 ; i < *n ; i++) {
            for(int j = 0 ; j < *m ; j++) {
                scanf("%d", *a_mat + i * (*m) + j);
            }
        }
        for(int i = 0 ; i < *m ; i++) {
            for(int j = 0 ; j < *l ; j++) {
                scanf("%d", *b_mat + i * (*l) + j);
            }
        }
    }

    MPI_Bcast(*a_mat, *n * (*m), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(*b_mat, *m * (*l), MPI_INT, 0, MPI_COMM_WORLD);
}

void matrix_multiply(const int n, const int m, const int l, const int *a_mat, const int *b_mat) {
    // MPI Init
    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    /**************************** init ************************************/
    const int NRA = n;
    const int NCA = m;
    const int NCB = l;
    const int FROM_MASTER = 0;  
    const int FROM_WORKER = 1;
    int offset; 
    int rows;
    int *c_mat = (int*)malloc(sizeof(int) * NRA * NCB);
    /**************************** master task ************************************/
    if (world_rank == 0) {
        /* Send matrix data to the worker tasks */
        int averow = NRA / world_size;
        int extra = NRA % world_size;
        offset = 0;
        for (int dest = 1; dest < world_size; dest++){
            rows = (dest <= extra) ? averow+1 : averow;   
            MPI_Send(&offset, 1, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
            offset += rows;
        }  
        /* Receive results from worker tasks */
        for (int source = 1; source < world_size; source++) {
            MPI_Recv(&offset, 1, MPI_INT, source, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&rows, 1, MPI_INT, source, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(c_mat + offset * NCB, rows * NCB, MPI_INT, source, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        /* Print results */
        // printf("******************************************************\n");
        // printf("Result Matrix:\n");
        for (int i=0; i < NRA; i++){
            for (int j=0; j < NCB; j++) 
                printf("%d ", *(c_mat + i * NCB + j));
            printf("\n"); 
        }
        // printf("\n******************************************************\n");
        // printf("Done.\n");
    }
    /**************************** worker task ************************************/
    if (world_rank > 0) {
        MPI_Recv(&offset, 1, MPI_INT, 0, FROM_MASTER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&rows, 1, MPI_INT, 0, FROM_MASTER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        const int *a;
        const int *b;
        int *ptr_c;
        for (int k = 0; k < NCB; k++) {
            for (int i = offset; i < rows + offset; i++) {
                ptr_c = c_mat + i * NCB + k;
                *ptr_c = 0;
                for (int j = 0; j < NCA; j++) {
                    a = a_mat + i * NCA + j;
                    b = b_mat + j * NCB + k;
                    *ptr_c += *a * (*b);
                }
            }
        }
        MPI_Send(&offset, 1, MPI_INT, 0, FROM_WORKER, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, 0, FROM_WORKER, MPI_COMM_WORLD);
        MPI_Send(c_mat + offset * NCB, rows * NCB, MPI_INT, 0, FROM_WORKER, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

void destruct_matrices(int *a_mat, int *b_mat){
    free(a_mat);
    free(b_mat);
}