#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Parámetros de simulación (ajustables)
#define WIDTH 512
#define HEIGHT 512
#define ITERATIONS 1000
#define OUTPUT_INTERVAL 100
#define PROB_DIVISION 0.75
#define PROB_MIGRATION 0.25
#define PROB_DEATH 0.05
#define HALO 1

// Estados de las celdas
#define EMPTY 0
#define CELL 1

// Variables globales
int **grid;
int local_width, local_height;
int rank, size;
MPI_Comm cart_comm;
int dims[2], coords[2], neighbors[4];
int block_width, block_height;

// Prototipos de funciones
void init_topology();
void init_grid();
void exchange_halo();
void simulate_step();
void try_migration(int i, int j);
void try_division(int i, int j);
void write_pgm(int iter);
int count_cells();
void free_resources();

// ===================== FUNCIÓN PRINCIPAL =====================
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Inicializar topología y malla
    init_topology();
    init_grid();
    
    // Archivo de métricas (solo rank 0)
    FILE *metrics = NULL;
    if (rank == 0) {
        metrics = fopen("cancer_metrics.csv", "w");
        fprintf(metrics, "Iteration,CellCount\n");
    }
    
    // Bucle principal de simulación
    for (int iter = 0; iter < ITERATIONS; iter++) {
        exchange_halo();
        simulate_step();
        
        // Salida periódica
        if (iter % OUTPUT_INTERVAL == 0) {
            int cell_count = count_cells();
            if (rank == 0) {
                fprintf(metrics, "%d,%d\n", iter, cell_count);
                fflush(metrics);
            }
            write_pgm(iter);
        }
    }
    
    // Liberar recursos y finalizar
    if (rank == 0) fclose(metrics);
    free_resources();
    MPI_Finalize();
    return 0;
}

// ===================== INICIALIZACIÓN =====================
void init_topology() {
    // Crear topología cartesiana 2D
    dims[0] = dims[1] = 0;
    MPI_Dims_create(size, 2, dims);
    
    int periods[2] = {0, 0};  // No periódica
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    
    // Calcular dimensiones locales
    block_width = WIDTH / dims[1];
    block_height = HEIGHT / dims[0];
    local_width = block_width + 2 * HALO;
    local_height = block_height + 2 * HALO;
    
    // Obtener vecinos
    MPI_Cart_shift(cart_comm, 0, 1, &neighbors[0], &neighbors[1]);  // Arriba/Abajo
    MPI_Cart_shift(cart_comm, 1, 1, &neighbors[2], &neighbors[3]);  // Izquierda/Derecha
}

void init_grid() {
    // Asignar memoria para la malla local (con halo)
    grid = (int**)malloc(local_height * sizeof(int*));
    for (int i = 0; i < local_height; i++) {
        grid[i] = (int*)malloc(local_width * sizeof(int));
        for (int j = 0; j < local_width; j++) {
            grid[i][j] = EMPTY;
        }
    }
    
    // Inicializar semilla aleatoria (única por proceso)
    srand(time(NULL) + rank);
    
    // Colocar célula inicial en el centro global (solo en el proceso correspondiente)
    int center_x = WIDTH / 2;
    int center_y = HEIGHT / 2;
    
    if (coords[0] == center_y / block_height && 
        coords[1] == center_x / block_width) {
        int local_y = center_y % block_height + HALO;
        int local_x = center_x % block_width + HALO;
        grid[local_y][local_x] = CELL;
    }
}

// ===================== COMUNICACIÓN PARALELA =====================
void exchange_halo() {
    MPI_Request reqs[8];
    MPI_Status stats[8];
    int count = 0;
    
    // Enviar y recibir bordes: ARRIBA <-> ABAJO
    if (neighbors[0] != MPI_PROC_NULL) {
        MPI_Isend(&grid[HALO][HALO], block_width, MPI_INT, neighbors[0], 0, cart_comm, &reqs[count++]);
        MPI_Irecv(&grid[0][HALO], block_width, MPI_INT, neighbors[0], 1, cart_comm, &reqs[count++]);
    }
    
    if (neighbors[1] != MPI_PROC_NULL) {
        MPI_Isend(&grid[local_height - HALO - 1][HALO], block_width, MPI_INT, neighbors[1], 1, cart_comm, &reqs[count++]);
        MPI_Irecv(&grid[local_height - 1][HALO], block_width, MPI_INT, neighbors[1], 0, cart_comm, &reqs[count++]);
    }
    
    // Enviar y recibir bordes: IZQUIERDA <-> DERECHA
    if (neighbors[2] != MPI_PROC_NULL) {
        for (int i = HALO; i < local_height - HALO; i++) {
            MPI_Isend(&grid[i][HALO], 1, MPI_INT, neighbors[2], 2, cart_comm, &reqs[count++]);
            MPI_Irecv(&grid[i][0], 1, MPI_INT, neighbors[2], 3, cart_comm, &reqs[count++]);
        }
    }
    
    if (neighbors[3] != MPI_PROC_NULL) {
        for (int i = HALO; i < local_height - HALO; i++) {
            MPI_Isend(&grid[i][local_width - HALO - 1], 1, MPI_INT, neighbors[3], 3, cart_comm, &reqs[count++]);
            MPI_Irecv(&grid[i][local_width - 1], 1, MPI_INT, neighbors[3], 2, cart_comm, &reqs[count++]);
        }
    }
    
    MPI_Waitall(count, reqs, stats);
}

// ===================== LÓGICA DE SIMULACIÓN =====================
void simulate_step() {
    // Actualizar solo celdas internas (excluyendo halo)
    for (int i = HALO; i < local_height - HALO; i++) {
        for (int j = HALO; j < local_width - HALO; j++) {
            if (grid[i][j] == CELL) {
                double r = (double)rand() / RAND_MAX;
                
                // 1. Muerte celular
                if (r < PROB_DEATH) {
                    grid[i][j] = EMPTY;
                } 
                // 2. Migración
                else if (r < PROB_DEATH + PROB_MIGRATION) {
                    try_migration(i, j);
                } 
                // 3. División
                else if (r < PROB_DEATH + PROB_MIGRATION + PROB_DIVISION) {
                    try_division(i, j);
                }
            }
        }
    }
}

void try_migration(int i, int j) {
    // Dirección aleatoria (8-vecinos)
    int dx = rand() % 3 - 1;
    int dy = rand() % 3 - 1;
    if (dx == 0 && dy == 0) return;
    
    int ni = i + dy;
    int nj = j + dx;
    
    // Verificar límites (incluyendo halo)
    if (ni < 0 || ni >= local_height || nj < 0 || nj >= local_width) return;
    
    // Mover si la celda destino está vacía
    if (grid[ni][nj] == EMPTY) {
        grid[i][j] = EMPTY;
        grid[ni][nj] = CELL;
    }
}

void try_division(int i, int j) {
    // Dirección aleatoria para nueva célula
    int dx = rand() % 3 - 1;
    int dy = rand() % 3 - 1;
    if (dx == 0 && dy == 0) return;
    
    int ni = i + dy;
    int nj = j + dx;
    
    // Verificar límites
    if (ni < 0 || ni >= local_height || nj < 0 || nj >= local_width) return;
    
    // Crear nueva célula si está vacío
    if (grid[ni][nj] == EMPTY) {
        grid[ni][nj] = CELL;
    }
}

// ===================== SALIDA DE RESULTADOS =====================
void write_pgm(int iter) {
    // Recopilar datos en rank 0
    int *global_grid = NULL;
    int *local_buffer = (int*)malloc(block_width * block_height * sizeof(int));
    
    // Empaquetar datos locales (sin halo)
    int idx = 0;
    for (int i = HALO; i < local_height - HALO; i++) {
        for (int j = HALO; j < local_width - HALO; j++) {
            local_buffer[idx++] = grid[i][j];
        }
    }
    
    // Preparar para recolección en rank 0
    int recvcounts[size];
    int displs[size];
    int block_size = block_width * block_height;
    
    for (int i = 0; i < size; i++) {
        recvcounts[i] = block_size;
        displs[i] = i * block_size;
    }
    
    if (rank == 0) {
        global_grid = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
    }
    
    // Recolectar todos los bloques
    MPI_Gatherv(local_buffer, block_size, MPI_INT,
               global_grid, recvcounts, displs, MPI_INT,
               0, cart_comm);
    
    // Escribir imagen PGM (solo rank 0)
    if (rank == 0) {
        char filename[50];
        sprintf(filename, "cancer_iter_%04d.pgm", iter);
        FILE *pgm = fopen(filename, "wb");
        
        // Encabezado PGM
        fprintf(pgm, "P5\n%d %d\n255\n", WIDTH, HEIGHT);
        
        // Convertir a formato binario
        unsigned char *img = (unsigned char*)malloc(WIDTH * HEIGHT);
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            img[i] = (global_grid[i] == CELL) ? 0 : 255;  // Células=negro, Fondo=blanco
        }
        
        fwrite(img, 1, WIDTH * HEIGHT, pgm);
        fclose(pgm);
        free(img);
        free(global_grid);
    }
    
    free(local_buffer);
}

int count_cells() {
    int count = 0;
    for (int i = HALO; i < local_height - HALO; i++) {
        for (int j = HALO; j < local_width - HALO; j++) {
            if (grid[i][j] == CELL) count++;
        }
    }
    
    int global_count;
    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, cart_comm);
    return global_count;
}

// ===================== LIMPIEZA =====================
void free_resources() {
    for (int i = 0; i < local_height; i++) {
        free(grid[i]);
    }
    free(grid);
}
