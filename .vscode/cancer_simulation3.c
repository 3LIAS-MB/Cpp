#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

const int GRID_SIZE = 200;
const int MAX_ITER = 5000;
const double FRACTAL_THRESHOLD = 0.65;
const double GROWTH_PROB = 0.1;

// Función para generar un patrón fractal inicial
void initializeFractalGrid(std::vector<std::vector<bool>>& grid) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Patrón fractal simple basado en función trigonométrica
            double value = 0.5 * (sin(i * 0.1) + cos(j * 0.1) + sin(sqrt(i*i + j*j) * 0.05));
            grid[i][j] = (value > FRACTAL_THRESHOLD);
        }
    }
    // Semilla inicial de cáncer en el centro
    grid[GRID_SIZE/2][GRID_SIZE/2] = true;
}

// Función para simular el crecimiento del cáncer
void simulateCancerGrowth(std::vector<std::vector<bool>>& grid, int rank, int size) {
    int rows_per_proc = GRID_SIZE / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? GRID_SIZE : start_row + rows_per_proc;
    
    std::vector<std::vector<bool>> new_grid(GRID_SIZE, std::vector<bool>(GRID_SIZE, false));
    
    for (int iter = 0; iter < MAX_ITER; iter++) {
        // Comunicar bordes entre procesos
        std::vector<bool> send_top(GRID_SIZE), send_bottom(GRID_SIZE);
        std::vector<bool> recv_top(GRID_SIZE), recv_bottom(GRID_SIZE);
        
        if (rank > 0) {
            for (int j = 0; j < GRID_SIZE; j++) 
                send_top[j] = grid[start_row][j];
            MPI_Send(send_top.data(), GRID_SIZE, MPI_CXX_BOOL, rank-1, 0, MPI_COMM_WORLD);
        }
        
        if (rank < size-1) {
            MPI_Recv(recv_bottom.data(), GRID_SIZE, MPI_CXX_BOOL, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < GRID_SIZE; j++) 
                grid[end_row][j] = recv_bottom[j];
        }
        
        if (rank < size-1) {
            for (int j = 0; j < GRID_SIZE; j++) 
                send_bottom[j] = grid[end_row-1][j];
            MPI_Send(send_bottom.data(), GRID_SIZE, MPI_CXX_BOOL, rank+1, 0, MPI_COMM_WORLD);
        }
        
        if (rank > 0) {
            MPI_Recv(recv_top.data(), GRID_SIZE, MPI_CXX_BOOL, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < GRID_SIZE; j++) 
                grid[start_row-1][j] = recv_top[j];
        }
        
        // Actualizar células en el segmento asignado
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[i][j]) {
                    new_grid[i][j] = true;  // La célula cancerosa permanece
                    
                    // Crecimiento fractal: las células vecinas pueden volverse cancerosas
                    for (int di = -1; di <= 1; di++) {
                        for (int dj = -1; dj <= 1; dj++) {
                            if (di == 0 && dj == 0) continue;
                            
                            int ni = i + di;
                            int nj = j + dj;
                            
                            if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE) {
                                // Regla de crecimiento basada en probabilidad y patrón fractal
                                if (!grid[ni][nj] && (rand() / (double)RAND_MAX) < GROWTH_PROB) {
                                    double fractal_val = 0.5 * (sin(ni * 0.1) + cos(nj * 0.1) + 
                                                      sin(sqrt(ni*ni + nj*nj) * 0.05);
                                    if (fractal_val > FRACTAL_THRESHOLD) {
                                        new_grid[ni][nj] = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Combinar los resultados de todos los procesos
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[i][j] = new_grid[i][j];
            }
        }
        
        // Sincronización periódica
        if (iter % 100 == 0) {
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
}

// Función para guardar la imagen resultante en formato PGM
void saveToPGM(const std::vector<std::vector<bool>>& grid, const std::string& filename) {
    std::ofstream pgmFile(filename);
    pgmFile << "P2\n" << GRID_SIZE << " " << GRID_SIZE << "\n255\n";
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            pgmFile << (grid[i][j] ? "0 " : "255 ");
        }
        pgmFile << "\n";
    }
    pgmFile.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    srand(time(NULL) + rank);  // Diferente semilla para cada proceso
    
    std::vector<std::vector<bool>> grid(GRID_SIZE, std::vector<bool>(GRID_SIZE, false));
    
    // Solo el proceso raíz inicializa el grid completo
    if (rank == 0) {
        initializeFractalGrid(grid);
    }
    
    // Distribuir el grid inicial a todos los procesos
    for (int i = 0; i < GRID_SIZE; i++) {
        MPI_Bcast(grid[i].data(), GRID_SIZE, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
    }
    
    // Simular el crecimiento del cáncer
    simulateCancerGrowth(grid, rank, size);
    
    // Recolectar resultados en el proceso raíz
    std::vector<std::vector<bool>> finalGrid;
    if (rank == 0) {
        finalGrid = std::vector<std::vector<bool>>(GRID_SIZE, std::vector<bool>(GRID_SIZE));
    }
    
    for (int i = 0; i < GRID_SIZE; i++) {
        if (rank == 0) {
            // El raíz recibe segmentos de otros procesos
            for (int p = 1; p < size; p++) {
                int start = p * (GRID_SIZE / size);
                int end = (p == size-1) ? GRID_SIZE : start + (GRID_SIZE / size);
                if (i >= start && i < end) {
                    MPI_Recv(finalGrid[i].data(), GRID_SIZE, MPI_CXX_BOOL, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
            // Copiar su propio segmento
            int start = 0;
            int end = GRID_SIZE / size;
            if (i < end) {
                finalGrid[i] = grid[i];
            }
        } else {
            // Los otros procesos envían sus segmentos
            int start = rank * (GRID_SIZE / size);
            int end = (rank == size-1) ? GRID_SIZE : start + (GRID_SIZE / size);
            if (i >= start && i < end) {
                MPI_Send(grid[i].data(), GRID_SIZE, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    // Guardar resultado como imagen
    if (rank == 0) {
        saveToPGM(finalGrid, "cancer_simulation.pgm");
        std::cout << "Simulacion completada. Resultados guardados en cancer_simulation.pgm" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}
