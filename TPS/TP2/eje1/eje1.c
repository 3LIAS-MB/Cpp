#include <omp.h>
#include <stdio.h>
int main()
{
    int threads;
    threads = omp_get_num_threads();
    printf("Total %d Threads\n", threads);
#pragma omp parallel
    {
        threads = omp_get_num_threads();
    }
    printf("Total %d Threads\n", threads);
}
