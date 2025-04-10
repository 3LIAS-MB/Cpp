#include <omp.h>
#include <stdio.h>
int main()
{
    int threads,
        id = 100;
#pragma omp parallel private(id)
    {
        id = omp_get_thread_num();
        threads = omp_get_num_threads();
        printf("Thread ID %d de %d\n", id, threads);
    }
    printf("ID %d\n", id);
}