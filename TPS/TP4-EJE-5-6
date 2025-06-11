#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <mpi.h>

// Función para verificar si un número es primo
bool es_primo(int numero, const std::vector<int>& primos_anteriores) {
    const long long numero_ll = static_cast<long long>(numero);
    for (size_t i = 1; i < primos_anteriores.size(); ++i) {
        const int p = primos_anteriores[i];
        if (static_cast<long long>(p) * p > numero_ll) {
            break;
        }
        if (numero % p == 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            std::cerr << "Uso: " << argv[0] << " n\n";
        }
        MPI_Finalize();
        return 1;
    }

    int n = std::atoi(argv[1]);
    if (n <= 0) {
        if (rank == 0) {
            std::cerr << "n debe ser un entero positivo\n";
        }
        MPI_Finalize();
        return 1;
    }

    // Solo el proceso 0 imprime el encabezado
    if (rank == 0) {
        std::cout << "Buscando los primeros " << n << " numeros primos usando " << size << " procesos...\n";
    }

    auto inicio = std::chrono::high_resolution_clock::now();

    std::vector<int> primos_globales;
    std::vector<int> primos_locales;
    
    // El proceso 0 maneja los primeros primos necesarios para las verificaciones
    if (rank == 0) {
        primos_globales.reserve(n);
        primos_globales.push_back(2);
        
        // Generamos suficientes primos para que todos los procesos puedan verificar
        int numero = 3;
        while (primos_globales.size() < std::min(n, 100)) { // Generamos al menos 100 primos para dividir el trabajo
            if (es_primo(numero, primos_globales)) {
                primos_globales.push_back(numero);
            }
            numero += 2;
        }
    }

    // Broadcast de los primos base a todos los procesos
    int num_primos_base;
    if (rank == 0) {
        num_primos_base = primos_globales.size();
    }
    MPI_Bcast(&num_primos_base, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        primos_globales.resize(num_primos_base);
    }
    MPI_Bcast(primos_globales.data(), num_primos_base, MPI_INT, 0, MPI_COMM_WORLD);

    // Distribución del trabajo
    int inicio_num = 0, fin_num = 0;
    if (rank == 0) {
        // El proceso 0 ya tiene algunos primos
        inicio_num = primos_globales.back() + 2;
        
        // Calculamos el rango de números que cada proceso verificará
        int rango = (n * 10 / size); // Estimación del rango necesario
        for (int i = 1; i < size; i++) {
            inicio_num = (i == 1) ? primos_globales.back() + 2 : fin_num + 2;
            fin_num = inicio_num + rango;
            MPI_Send(&inicio_num, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&fin_num, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&inicio_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&fin_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Cada proceso busca primos en su rango asignado
    int numero = inicio_num;
    while (primos_globales.size() < n && numero <= fin_num) {
        if (es_primo(numero, primos_globales)) {
            primos_locales.push_back(numero);
        }
        numero += 2;
    }

    // Recolectamos todos los primos encontrados en el proceso 0
    std::vector<int> todos_primos;
    if (rank == 0) {
        todos_primos = primos_globales;
        todos_primos.insert(todos_primos.end(), primos_locales.begin(), primos_locales.end());
        
        // Recibir primos de otros procesos
        for (int i = 1; i < size; i++) {
            int num_primos_recibir;
            MPI_Recv(&num_primos_recibir, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            std::vector<int> primos_recibidos(num_primos_recibir);
            MPI_Recv(primos_recibidos.data(), num_primos_recibir, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            todos_primos.insert(todos_primos.end(), primos_recibidos.begin(), primos_recibidos.end());
        }
        
        // Ordenamos y seleccionamos los primeros n primos
        std::sort(todos_primos.begin(), todos_primos.end());
        if (todos_primos.size() > n) {
            todos_primos.resize(n);
        }
    } else {
        int num_primos_enviar = primos_locales.size();
        MPI_Send(&num_primos_enviar, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(primos_locales.data(), num_primos_enviar, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    auto fin = std::chrono::high_resolution_clock::now();
    
    // Solo el proceso 0 muestra los resultados
    if (rank == 0) {
        auto duracion_ms = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
        std::chrono::duration<double> duracion_sg = fin - inicio;

        std::cout << std::fixed << std::setprecision(6);
        std::cout << n << " primos generados en:\n";
        std::cout << " - " << duracion_ms.count() << " milisegundos\n";
        std::cout << " - " << duracion_sg.count() << " segundos\n";
        
        // Opcional: mostrar los primeros y últimos primos encontrados
        if (n <= 20) {
            std::cout << "Primos encontrados: ";
            for (int p : todos_primos) {
                std::cout << p << " ";
            }
            std::cout << "\n";
        } else {
            std::cout << "Algunos primos encontrados: ";
            for (int i = 0; i < 5; i++) {
                std::cout << todos_primos[i] << " ";
            }
            std::cout << "... ";
            for (int i = todos_primos.size() - 5; i < todos_primos.size(); i++) {
                std::cout << todos_primos[i] << " ";
            }
            std::cout << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
