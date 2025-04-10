#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace std;
using namespace std::chrono;

double calcularPi(int n) {
    double suma = 0.0;
    double ancho = 1.0 / n;
    
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