// El ejercicio demuestra conceptos clave de programación paralela:
// creación de hilos, sincronización con barreras, diferenciación de roles	
// entre hilos y gestión de tiempos desiguales en un entorno concurrente.

#include <omp.h>       // Para OpenMP
#include <stdio.h>     // Para printf
#include <unistd.h>    // Para usleep()

#define SEGUNDOS1 1000000
#define SEGUNDOS4 4000000

double funcion(int tiempo){
	usleep(tiempo);
	return(tiempo);
}
	
int main() {
	int threads, x, id;
	// Llave obligatoria (múltiples instrucciones)
	#pragma omp parallel private(id)
	{ 	
		id = omp_get_thread_num(); // omp_set_num_threads
		if (id == 0){
			printf("Soy el nodo master y me ejecuto 1 segundo\n");	
			x = funcion(SEGUNDOS1);
		}else{			
			printf("Soy el nodo %d y me ejecuto 4 segundos\n",  id);	
			x = funcion(SEGUNDOS4);
		}
		#pragma omp barrier // Sin llaves
		printf("----->Soy el nodo %d y termine la ejecucion\n",  id);	
	}
}

// gcc -fopenmp ejemploBarrier.c -o ejemploBarrier