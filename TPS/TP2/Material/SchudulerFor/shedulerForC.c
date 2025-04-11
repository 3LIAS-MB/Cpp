#include <omp.h>
#include <stdio.h>

int main() {        
    double vectorY[] = {12,51,11,21,12,13,23,34,100,45,56,11,10,22,21};
    int TAM = sizeof(vectorY) / sizeof(vectorY[0]); // TAM = 15
    int threads;

    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads(); 
    }
    printf("Cantidad de Threads: %d\n", threads);
    // 'for' indica que el bucleserá ejecutado en paralelo.
    // divide las iteraciones en varios hilos, q se ejecutan
    // simultáneamente, aprovechando varios núcleos del procesador.

    // schedule -> Especifica cómo se distribuyen las iteraciones del bucle entre los hilos.
    // static -> divide las iteraciones en bloques de tamaño fijo
    // 1 -> Indica que cada bloque tendrá exactamente 1 iteración
    #pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < TAM; i++) {
        int id = omp_get_thread_num(); 
        printf("Thread %d: Y[%d] = %.2f\n", id, i, vectorY[i]);
    } 	
    return 0;
}

// gcc -fopenmp shedulerForC.c -o shedulerForC