// 1. Implementa en lenguaje C++ un programa que permita generar los n
//  primeros números primos, donde n se enviara como argumento al momento de
//  ejecutar el programa.

// -> Se pide:
// • Ejecute el programa con los siguientes valores de n: 100, 1000, 10000,
// 100000 y 400000. 
// • Represente en una tabla comparativa los tiempos de ejecución
// obtenidos para cada valor de n.
// • Comente acerca de los resultados obtenidos.

#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>

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
    primos.push_back(2);

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
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio).count();

    std::cout << n << " primos generados en " << duracion << " ms\n";

    return 0;
}

// -> cl / EHsc Prueba.cpp
// -> .\Prueba.exe
