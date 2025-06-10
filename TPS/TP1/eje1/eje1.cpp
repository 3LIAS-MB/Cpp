#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <iomanip>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " n\n";
        return 1;
    }

    int n = std::atoi(argv[1]);
    if (n <= 0) {
        std::cerr << "n debe ser un entero positivo\n";
        return 1;
    }

    auto inicio = std::chrono::high_resolution_clock::now();

    // Reserva y almacenamiento de primos
    std::vector<int> primos;
    primos.reserve(n);
    primos.push_back(2);

    // Bucle busqueda de primos
    int numero = 3;
    while (primos.size() < n) {
        bool es_primo = true;
        const long long numero_ll = static_cast<long long>(numero);
        for (size_t i = 1; i < primos.size(); ++i) {
            const int p = primos[i];
            if (static_cast<long long>(p) * p > numero_ll) {
                break;
            }
            if (numero % p == 0) {
                es_primo = false;
                break;
            }
        }
        if (es_primo) {
            primos.push_back(numero);
        }
        numero += 2;
    }

    auto fin = std::chrono::high_resolution_clock::now();
    
    // Calculamos ambas medidas de tiempo
    auto duracion_ms = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
    std::chrono::duration<double> duracion_sg = fin - inicio;

    // Configuramos la precisión para segundos
    std::cout << std::fixed << std::setprecision(6);
    
    // Mostramos ambos formatos de tiempo
    std::cout << n << " primos generados en:\n";
    std::cout << " - " << duracion_ms.count() << " milisegundos\n";
    std::cout << " - " << duracion_sg.count() << " segundos\n";

    return 0;
}
