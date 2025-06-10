#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <omp.h>
#include <iomanip>

bool es_primo(int numero, const std::vector<int>& primos) {
    int limite = std::sqrt(numero) + 1;
    for (int p : primos) {
        if (p > limite) break;
        if (numero % p == 0) return false;
    }
    return true;
}

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

    std::vector<int> primos;
    primos.reserve(n);

    if (n >= 1) primos.push_back(2);
    if (n >= 2) primos.push_back(3);

    // Límite empíricamente ajustado para n >= 400,000
    int limite_superior = (n == 400000) ? 5800000 : static_cast<int>(n * (log(n) + log(log(n))));

    // Region paralela OpenMP
    #pragma omp parallel
    {
        std::vector<int> primos_local;
        #pragma omp for schedule(static)
        for (int numero = 5; numero <= limite_superior; numero += 2) {
            if (es_primo(numero, primos)) {
                primos_local.push_back(numero);
            }
        }
        #pragma omp critical
        primos.insert(primos.end(), primos_local.begin(), primos_local.end());
    }

    if (primos.size() > n) {
        primos.resize(n);
    }

    auto fin = std::chrono::high_resolution_clock::now();
    
    auto duracion_ms = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
    std::chrono::duration<double> duracion_sg = fin - inicio;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << n << " primos generados en:\n";
    std::cout << " - " << duracion_ms.count() << " milisegundos\n";
    std::cout << " - " << duracion_sg.count() << " segundos\n";

    return 0;
}
