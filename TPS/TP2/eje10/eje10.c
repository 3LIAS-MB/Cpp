
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>   
#include <omp.h>

void intercambiar(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main() {
    clock_t inicio, fin;
    double tiempo_total;
    int n;
    printf("Ingrese el tamaño del vector: ");
    scanf("%i", &n);

    int vector[n];
    srand(time(NULL)); // Inicializar la semilla de los números aleatorios

    printf("El vector se genero de Manera Aleatoria con numeros Enteros:\nEl vector es el siguienteXXX");
    for (int i = 0; i < n; i++) {
        vector[i] = rand() % 1000000;
       // printf("%d ", vector[i]);//solo para ver
    }
    printf("\n");

    inicio = clock(); // Guardar el tiempo de inicio
    
	#pragma omp parallel for shared(vector)
		for (int i = 0; i < n - 1; i++) {
			// Encontrar el índice del mínimo elemento en el array
			int min_idx = i;
			for (int j = i + 1; j < n; j++) {
				if (vector[j] < vector[min_idx]) {
					min_idx = j;
				}
			}
			// Intercambiar el mínimo elemento con el elemento en la posición actual
			#pragma omp critical
			{
				intercambiar(&vector[min_idx], &vector[i]);
			}
		}


    fin = clock(); // Guardar el tiempo de fin
    
    tiempo_total = (double)(fin - inicio) / CLOCKS_PER_SEC;
    printf("\nTiempo de ejecución: %f segundos\n", tiempo_total);
    return 0;
}
