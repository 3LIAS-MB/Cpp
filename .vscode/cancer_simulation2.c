
/*
 * Simulación básica de crecimiento tumoral usando DLA (Diffusion-Limited Aggregation)
 * Paralelizado con MPI. 
 *
 * Requiere:
 *   - MPICH o OpenMPI instalado
 *   - Compilación: mpicc -o dla dla.c
 *   - Ejecución: mpirun -np 4 ./dla
 *
 * Salida:
 *   - Archivos PGM cada N iteraciones (nombre: frame_###.pgm)
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 512
#define HEIGHT 512
#define ITERATIONS 1000
#define OUTPUT_INTERVAL 100

#define DIVIDE_PROB 0.75
#define MIGRATE_PROB 0.25
#define DIE_PROB 0.05

#define ALIVE 1
#define DEAD 0

int is_valid(int x, int y) {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

void save_pgm(char* filename, unsigned char* grid) {
    FILE* f = fopen(filename, "w");
    fprintf(f, "P2\n%d %d\n1\n", WIDTH, HEIGHT);
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fprintf(f, "%d ", grid[i * WIDTH + j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_proc = HEIGHT / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank + 1) * rows_per_proc;

    unsigned char* grid = calloc(WIDTH * rows_per_proc, sizeof(unsigned char));
    unsigned char* full_grid = NULL;

    if (rank == 0) {
        full_grid = calloc(WIDTH * HEIGHT, sizeof(unsigned char));
        full_grid[(HEIGHT / 2) * WIDTH + (WIDTH / 2)] = ALIVE; // Semilla inicial
    }

    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Broadcast del estado completo a todos los procesos
        MPI_Scatter(full_grid, WIDTH * rows_per_proc, MPI_UNSIGNED_CHAR,
                    grid, WIDTH * rows_per_proc, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        // Procesar la subgrilla
        for (int i = 1; i < rows_per_proc - 1; i++) {
            for (int j = 1; j < WIDTH - 1; j++) {
                int idx = i * WIDTH + j;
                if (grid[idx] == ALIVE) {
                    continue;
                }

                int neighbors =
                    grid[(i - 1) * WIDTH + j] +
                    grid[(i + 1) * WIDTH + j] +
                    grid[i * WIDTH + (j - 1)] +
                    grid[i * WIDTH + (j + 1)];

                if (neighbors > 0) {
                    double r = (double)rand() / RAND_MAX;
                    if (r < DIVIDE_PROB) {
                        grid[idx] = ALIVE;
                    }
                }
            }
        }

        // Recolectar la grilla procesada
        MPI_Gather(grid, WIDTH * rows_per_proc, MPI_UNSIGNED_CHAR,
                   full_grid, WIDTH * rows_per_proc, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        // Guardar imagen cada N pasos
        if (rank == 0 && iter % OUTPUT_INTERVAL == 0) {
            char filename[64];
            sprintf(filename, "frame_%04d.pgm", iter);
            save_pgm(filename, full_grid);
            printf("Iteración %d guardada en %s\n", iter, filename);
        }
    }

    free(grid);
    if (rank == 0) free(full_grid);

    MPI_Finalize();
    return 0;
}
