#include <omp.h>
#include <stdio.h>
#include <unistd.h>  // Para usleep()

#define SEGUNDOS3 3000000
#define SEGUNDOS4 4000000
#define SEGUNDOS6 6000000

double funcion(int tiempo) {
    usleep(tiempo);
    return tiempo;
}

int main() {
    double x, y, z, suma;
    #pragma omp parallel
    {
        int id = omp_get_thread_num();  // Variable local (sin race condition)
        if (id == 0) {
            printf("\nEjecucion de secciones en paralelo con %d hilos\n", omp_get_num_threads());
        }
        #pragma omp sections
        {
            #pragma omp section
            { x = funcion(SEGUNDOS3); }
            #pragma omp section
            { y = funcion(SEGUNDOS6); }
            #pragma omp section
            { z = funcion(SEGUNDOS4); }
        }
    }
    suma = x + y + z;
    printf("\nResultados: x=%.2fs, y=%.2fs, z=%.2fs --> suma=%.2fs\n", 
           x/1e6, y/1e6, z/1e6, suma/1e6);
    return 0;
}