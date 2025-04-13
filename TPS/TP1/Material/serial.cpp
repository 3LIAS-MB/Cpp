#include "common.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv) {
    if (find_option(argc, argv, "-h") >= 0) {
        printf("Opciones:\n");
        printf("-h : Muestra ayuda\n");
        printf("-n <int> : Número de partículas (default: 1000)\n");
        printf("-o <file> : Archivo de salida (default: salida.txt)\n");
        return 0;
    }

    int n = read_int(argc, argv, "-n", 1000);
    std::string output_file = read_string(argc, argv, "-o", "salida.txt");

    FILE *fsave = fopen(output_file.c_str(), "w");
    Particle *particles = new Particle[n];
    set_size(n);
    init_particles(n, particles);

    // Simulación
    double start_time = read_timer();
    for (int step = 0; step < NSTEPS; step++) {
        // Resetear aceleraciones
        for (int i = 0; i < n; i++) {
            particles[i].ax = 0;
            particles[i].ay = 0;
        }

        // Calcular fuerzas (O(n²))
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                apply_force(particles[i], particles[j]);
            }
        }

        // Mover partículas
        for (int i = 0; i < n; i++) {
            move(particles[i]);
        }

        // Guardar resultados
        if (fsave && (step % SAVEFREQ == 0)) {
            save(fsave, n, particles);
        }
    }
    double simulation_time = read_timer() - start_time;

    printf("n = %d, tiempo = %g segundos\n", n, simulation_time);

    delete[] particles;
    if (fsave) fclose(fsave);
    return 0;
}