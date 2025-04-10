#include <omp.h>
#include <stdio.h>

#define SEGUNDOS3 3000000
#define SEGUNDOS4 4000000
#define SEGUNDOS6 6000000

double funcion(int tiempo){
	usleep(tiempo);
	return(tiempo);
}

main (){
	int id=-1;
	double x, y, z, suma;
	#pragma omp parallel private(id)
	{
		#ifdef _OPENMP
			id = omp_get_thread_num();			 
			if (id==0) printf("\nEjecucion de secciones en paralelo con %d hilos\n", omp_get_num_threads());
		#endif
		#pragma omp sections
		{
			#pragma omp section
			{				
				printf("La seccion 1  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS3/1000000, id);
				x = funcion(SEGUNDOS3);
				printf("La seccion 1 finalizo\n");  		
			}
			#pragma omp section
			{
				printf("La seccion 2  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS6/1000000, id);
				y = funcion(SEGUNDOS6);			
				printf("La seccion 2 finalizo\n");  		
			}
			#pragma omp section
			{
				printf("La seccion 3  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS4/1000000, id);
				z = funcion(SEGUNDOS4);
				printf("La seccion 3 finalizo\n"); 
			}
		}
	}
	suma = x + y + z;
	printf ("\n Resultados x = %.2f y = %.2f z =%.2f --->  suma = %.2f\n", x, y, z, suma/1000000);
} 
