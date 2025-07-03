#include <omp.h>
#include <stdio.h>
#include <unistd.h>    // Para usleep()

#define SEGUNDOS1 1000000
#define SEGUNDOS5 5000000


double funcion(int tiempo){
	usleep(tiempo);
	return(tiempo);
}
	
int main (){
	int id;
	double x, y;
	// Fija explícitamente 2 hilos para toda la región paralela.
	omp_set_num_threads(2);	
	#pragma omp parallel private(id)
	{
		id = omp_get_thread_num();
		#pragma omp sections
		{
			#pragma omp section 
			{				
				printf("La seccion 1  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS1/1000000, id);
				x = funcion(SEGUNDOS1);
			}
			#pragma omp section
			{
				printf("La seccion 2  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS5/1000000, id);
				y = funcion(SEGUNDOS5);			
			}			
		}
		printf("----->Soy el nodo %d y termine la ejecucion de la seccion\n",  id);
	}
	return 0;
}

// dgcc -fopenmp -Wall ejemploNowait.c -o ejemploNowait
// El flag -Wall activa todas las advertencias para detectar prácticas inseguras. ✅