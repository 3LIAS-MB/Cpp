#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define SEGUNDOS3 3000000
#define SEGUNDOS4 4000000
#define SEGUNDOS6 6000000

double funcion(int tiempo)
{
	usleep(tiempo);
	return (tiempo);
}

int main()
{
	int id = -1;
	double x, y, z, suma;
#pragma omp parallel private(id)
	{
// `#ifdef` y `#endif` son directivas del preprocesador en C y C++.
//  Estas directivas no son parte del lenguaje en sí, sino que son
//  instrucciones para el **preprocesador**, una etapa que ocurre
//  antes de la compilación del código. Estas directivas permiten incluir
//  o excluir partes del código dependiendo de si una macro está definida.

// __OPENMP_ es una macro que se define automáticamente por el compilador
//  si se está utilizando OpenMP. Esto permite que el código dentro de
//  `#ifdef _OPENMP` solo se compile si OpenMP está habilitado.
//  Esto es útil para escribir código que puede ser compilado tanto
//  con OpenMP como sin OpenMP, permitiendo así la portabilidad del código.

// Propósito: Permite detectar si el código se está compilando con soporte para OpenMP.
// Valor: Es un número que representa la versión de OpenMP (ej: 201511 para OpenMP 4.5).
#ifdef _OPENMP
		id = omp_get_thread_num();
		if (id == 0)
			printf("\nEjecucion de secciones en paralelo con %d hilos\n", omp_get_num_threads());
#endif
#pragma omp sections
		{
#pragma omp section
			{
				printf("La seccion 1  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS3 / 1000000, id);
				x = funcion(SEGUNDOS3);
				printf("La seccion 1 finalizo\n");
			}
#pragma omp section
			{
				printf("La seccion 2  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS6 / 1000000, id);
				y = funcion(SEGUNDOS6);
				printf("La seccion 2 finalizo\n");
			}
#pragma omp section
			{
				printf("La seccion 3  se ejecutara %d segundos en el Thread %d\n", SEGUNDOS4 / 1000000, id);
				z = funcion(SEGUNDOS4);
				printf("La seccion 3 finalizo\n");
			}
		}
	}
	suma = x + y + z;
	printf("\n Resultados x = %.2f y = %.2f z =%.2f --->  suma = %.2f\n", x, y, z, suma / 1000000);
	return 0;
}
