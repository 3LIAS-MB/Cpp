#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <omp.h>  // Incluimos OpenMP

using namespace std;
using namespace std::chrono;

double calcularPi(int n) {
    double suma = 0.0;
    double ancho = 1.0 / n;

    // Paralelizamos con reducción para evitar condiciones de carrera
    #pragma omp parallel for reduction(+:suma)
    for (int i = 0; i < n; ++i) {
        double x = (i + 0.5) * ancho;
        suma += 4.0 / (1.0 + x * x) * ancho;
    }

    return suma;
}

int main() {
    // Valores de n para probar
    int valores_n[] = {10, 100, 1000, 10000, 100000, 1000000, 10000000};
    int num_pruebas = sizeof(valores_n) / sizeof(valores_n[0]);
    
    cout << "---------------------------------------------------------------\n";
    cout << "n\t\tAproximación π\t\tError Absoluto\t\tTiempo (s)\n";
    cout << "---------------------------------------------------------------\n";
    
    for (int i = 0; i < num_pruebas; ++i) {
        int n = valores_n[i];
        
        auto inicio = high_resolution_clock::now();
        double pi_aproximado = calcularPi(n);
        auto fin = high_resolution_clock::now();
        
      duration<double> duracion = fin - inicio;
        double error_absoluto = abs(pi_aproximado - M_PI);
        
        cout << n << "\t\t" 
             << fixed << setprecision(10) << pi_aproximado << "\t"
             << scientific << setprecision(6) << error_absoluto << "\t"
             << fixed << setprecision(6) << duracion.count() << endl;
    }
    
    return 0;
}


10

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
