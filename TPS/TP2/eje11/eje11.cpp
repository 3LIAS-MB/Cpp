#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <omp.h>

using namespace std;
using namespace std::chrono;

double calcularPi(int n, int num_threads) {
    double suma = 0.0;
    double ancho = 1.0 / n;

    omp_set_num_threads(num_threads);  // Establecer número de hilos

    // Cálculo paralelo
    #pragma omp parallel for reduction(+:suma)
    for (int i = 0; i < n; ++i) {
        double x = (i + 0.5) * ancho;
        suma += 4.0 / (1.0 + x * x) * ancho;
    }

    return suma;
}

int main() {
    int num_threads;
    cout << "Ingresa la cantidad de hilos a utilizar: ";
    cin >> num_threads;
    

    int valores_n[] = {10, 100, 1000, 10000, 100000, 1000000, 10000000};
    int num_pruebas = sizeof(valores_n) / sizeof(valores_n[0]);

    cout << "---------------------------------------------------------------\n";
    cout << "n\t\tAproximación π\t\tError Absoluto\t\tTiempo (s)\n";
    cout << "---------------------------------------------------------------\n";

    for (int i = 0; i < num_pruebas; ++i) {
        int n = valores_n[i];

        auto inicio = high_resolution_clock::now();
        double pi_aproximado = calcularPi(n, num_threads);
        auto fin = high_resolution_clock::now();

        duration<double> duracion = fin - inicio;
        double error_absoluto = abs(pi_aproximado - M_PI);

        cout << n << "\t\t"
             << fixed << setprecision(10) << pi_aproximado << "\t"
             << scientific << setprecision(6) << error_absoluto << "\t"
             << fixed << setprecision(6) << duracion.count() << endl;
    }
    cout << "→ Usando " << num_threads << " hilos en esta ejecución.\n";   
    return 0;
}
