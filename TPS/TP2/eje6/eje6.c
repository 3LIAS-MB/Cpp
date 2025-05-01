#include <stdio.h>
#include <omp.h>
int main(int argc, char *argv[])
{
    int i, id;
    long int n = 30;
    long int suma = 0;
#pragma omp parallel for reduction(+ : suma) private(id)
    for (i = 0; i < n; i++) 
    {
        if ((i % 2) == 0)
        {
            suma = suma + i;
#ifdef _OPENMP
            id = omp_get_thread_num();
#endif
            printf("Suma parcial en el thread: %d es %f\n", id, suma);
        }
    }
    printf("Suma Total = %u\n", suma);
}