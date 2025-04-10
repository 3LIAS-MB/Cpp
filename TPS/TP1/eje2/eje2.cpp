// 2. Implemente en lenguaje C/C++ un programa que permita ordenar un vector de
// tamaño n, donde n se enviara como argumento al momento de ejecutar el
// programa.
// Se pide:
// • Ejecute el programa con los siguientes valores de n: 1000, 10000,
// 100000 y 400000.
// • Represente en una tabla comparativa los tiempos de ejecución obtenidos
// para cada valor de n.
// • Comente acerca de los resultados obtenidos.

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

vector<int> generarVectorAleatorio(int n) {
    vector<int> vec(n);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 1000000);
    
    for (int i = 0; i < n; ++i) {
        vec[i] = distrib(gen);
    }
    
    return vec;
}

void ordenarVector(vector<int>& vec) {
    sort(vec.begin(), vec.end());
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <tamaño del vector>" << endl;
        return 1;
    }
    
    int n = stoi(argv[1]);
    
    // Generar vector aleatorio
    auto inicio_gen = high_resolution_clock::now();
    vector<int> vec = generarVectorAleatorio(n);
    auto fin_gen = high_resolution_clock::now();
    auto duracion_gen_ms = duration_cast<milliseconds>(fin_gen - inicio_gen);
    duration<double> duracion_gen_s = fin_gen - inicio_gen;
    
    // Ordenar vector
    auto inicio_sort = high_resolution_clock::now();
    ordenarVector(vec);
    auto fin_sort = high_resolution_clock::now();
    auto duracion_sort_ms = duration_cast<milliseconds>(fin_sort - inicio_sort);
    duration<double> duracion_sort_s = fin_sort - inicio_sort;
    
    // Configurar precisión decimal
    cout << fixed << setprecision(6);
    
    // Mostrar resultados
    cout << "Tamaño del vector: " << n << endl;
    cout << "Tiempo generación: " << duracion_gen_ms.count() << " ms (" << duracion_gen_s.count() << " s)" << endl;
    cout << "Tiempo ordenación: " << duracion_sort_ms.count() << " ms (" << duracion_sort_s.count() << " s)" << endl;
    cout << "Tiempo total: " << (duracion_gen_ms + duracion_sort_ms).count() << " ms (" 
         << (duracion_gen_s + duracion_sort_s).count() << " s)" << endl;
    
    return 0;
}