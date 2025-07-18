#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <math.h>

#define MAX_VECINOS 10
#define MAX_LINEA 256
#define DIAS 120
#define PROB_MOV 0.7
#define PORC_TRANSMISION 0.02

typedef struct {
    double S, I, R;
    int N;
    int num_vecinos;
    int vecinos[MAX_VECINOS];
    double peak_infection;
    int peak_day;
    int first_infection_day;
} Region;

typedef struct {
    int dia;
    double S, I, R;
} Registro;

void leer_datos_region(const char* archivo, int rank, Region* reg, int size) {
    FILE* f = fopen(archivo, "r");
    if (!f) {
        fprintf(stderr, "No se pudo abrir el archivo %s\n", archivo);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char linea[MAX_LINEA];
    int actual = -1;
    int encontrado = 0;

    while (fgets(linea, sizeof(linea), f)) {
        if (linea[0] == '#' || strlen(linea) < 3) continue;
        actual++;
        if (actual == rank) {
            char* token = strtok(linea, " ");
            reg->N = atoi(token);
            reg->S = atof(strtok(NULL, " "));
            reg->I = atof(strtok(NULL, " "));
            reg->R = atof(strtok(NULL, " "));

            reg->peak_infection = reg->I;
            reg->peak_day = 0;
            reg->first_infection_day = (reg->I > 0) ? 0 : -1;

            reg->num_vecinos = 0;
            while ((token = strtok(NULL, " ")) != NULL && reg->num_vecinos < MAX_VECINOS) {
                int vecino = atoi(token);
                if (vecino >= 0 && vecino < size) {
                    reg->vecinos[reg->num_vecinos++] = vecino;
                } else {
                    fprintf(stderr, "Vecino inválido %d para proceso %d\n", vecino, rank);
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }
            }
            encontrado = 1;
            break;
        }
    }

    fclose(f);

    if (!encontrado) {
        fprintf(stderr, "No hay suficientes líneas en el archivo para el proceso %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

MPI_Datatype crear_tipo_registro() {
    MPI_Datatype tipo_registro;
    int bloques[2] = {1, 3};
    MPI_Aint desplazamientos[2];
    desplazamientos[0] = offsetof(Registro, dia);
    desplazamientos[1] = offsetof(Registro, S);
    MPI_Datatype tipos[2] = {MPI_INT, MPI_DOUBLE};

    MPI_Type_create_struct(2, bloques, desplazamientos, tipos, &tipo_registro);
    MPI_Type_commit(&tipo_registro);
    return tipo_registro;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s archivo_entrada.txt\n", argv[0]);
        return 1;
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Region region;
    leer_datos_region(argv[1], rank, &region, size);

    unsigned int seed = (unsigned)time(NULL) ^ (rank << 16);

    MPI_Datatype tipo_registro = crear_tipo_registro();
    Registro registros[DIAS + 1];

    double beta = 0.8, gamma = 0.1;

    double enviar[MAX_VECINOS] = {0};
    double recibir[MAX_VECINOS] = {0};
    MPI_Request req_send[MAX_VECINOS];
    MPI_Request req_recv[MAX_VECINOS];
    
    double total_comm_time = 0.0;
    double comm_start, comm_end;
    if (rank == 0) {
        system("clear"); // Limpiar consola
        printf("▶ Simulando %d días con %d procesos...\n", DIAS, size);
        fflush(stdout);
    }
    // Barrera para sincronizar inicio
    MPI_Barrier(MPI_COMM_WORLD);
    double t_local_inicio = MPI_Wtime();
    
    for (int dia = 0; dia <= DIAS; dia++) {
        registros[dia].dia = dia;
        registros[dia].S = region.S;
        registros[dia].I = region.I;
        registros[dia].R = region.R;

        if (region.I > region.peak_infection) {
            region.peak_infection = region.I;
            region.peak_day = dia;
        }

        double nuevos_infectados = beta * region.S * region.I / region.N;
        double nuevos_recuperados = gamma * region.I;
        
        region.S -= nuevos_infectados;
        region.I += nuevos_infectados - nuevos_recuperados;
        region.R += nuevos_recuperados;

        double total = region.S + region.I + region.R;
        if (fabs(total - region.N) > 1e-5) {
            double error = region.N - total;
            region.S += error * (region.S / total);
            region.I += error * (region.I / total);
            region.R += error * (region.R / total);
        }

        // ========== COMUNICACIÓN MEJORADA ==========
        comm_start = MPI_Wtime();
        
        double total_a_enviar = 0.0;
        if (region.I > 1.0 && (double)rand_r(&seed) / RAND_MAX < PROB_MOV) {
            total_a_enviar = region.I * PORC_TRANSMISION;
            if (total_a_enviar > region.I) total_a_enviar = region.I;
        }

        for (int i = 0; i < region.num_vecinos; i++) {
            enviar[i] = total_a_enviar / region.num_vecinos;
        }
        region.I -= total_a_enviar;

        // Post receives
        for (int i = 0; i < region.num_vecinos; i++) {
            MPI_Irecv(&recibir[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_recv[i]);
        }

        // Post sends
        for (int i = 0; i < region.num_vecinos; i++) {
            MPI_Isend(&enviar[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_send[i]);
        }

        // Esperar receives antes de procesar
        MPI_Waitall(region.num_vecinos, req_recv, MPI_STATUSES_IGNORE);
        for (int i = 0; i < region.num_vecinos; i++) {
            region.I += recibir[i];
            if (recibir[i] > 0 && region.first_infection_day == -1) {
                region.first_infection_day = dia;
            }
        }
        
        // Esperar sends para consistencia
        MPI_Waitall(region.num_vecinos, req_send, MPI_STATUSES_IGNORE);
        
        comm_end = MPI_Wtime();
        total_comm_time += (comm_end - comm_start);
        // ===========================================
    }

    // Sincronización final para tiempos precisos
    MPI_Barrier(MPI_COMM_WORLD);
    double t_local_fin = MPI_Wtime();
    double tiempo_total = t_local_fin - t_local_inicio;

    if (rank == 0) {
        FILE* f = fopen("resultados_global.csv", "w");
        if (!f) {
            fprintf(stderr, "No se pudo abrir el archivo de salida\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(f, "Region,Dia,S,I,R\n");
        for (int d = 0; d <= DIAS; d++) {
            fprintf(f, "%d,%d,%.2f,%.2f,%.2f\n", rank, d, registros[d].S, registros[d].I, registros[d].R);
        }

        for (int src = 1; src < size; src++) {
            Registro otros[DIAS + 1];
            MPI_Recv(otros, DIAS + 1, tipo_registro, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int d = 0; d <= DIAS; d++) {
                fprintf(f, "%d,%d,%.2f,%.2f,%.2f\n", src, otros[d].dia, otros[d].S, otros[d].I, otros[d].R);
            }
        }
        fclose(f);
    } else {
        MPI_Send(registros, DIAS + 1, tipo_registro, 0, 1, MPI_COMM_WORLD);
    }

    double metricas[3] = {region.peak_infection, (double)region.peak_day, (double)region.first_infection_day};
    double *todas_metricas = NULL;
    double *tiempos = NULL;
    double *comm_times = NULL;
    
    if (rank == 0) {
        todas_metricas = malloc(size * 3 * sizeof(double));
        tiempos = malloc(size * sizeof(double));
        comm_times = malloc(size * sizeof(double));
    }

    MPI_Gather(metricas, 3, MPI_DOUBLE, todas_metricas, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&tiempo_total, 1, MPI_DOUBLE, tiempos, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&total_comm_time, 1, MPI_DOUBLE, comm_times, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        FILE* resumen = fopen("resumen_epidemiologico.txt", "w");
        fprintf(resumen, "RESULTADOS EPIDEMIOLÓGICOS Y DE RENDIMIENTO\n");
        fprintf(resumen, "==========================================\n\n");
        
        fprintf(resumen, "Configuración de simulación:\n");
        fprintf(resumen, " - Días simulados: %d\n", DIAS);
        fprintf(resumen, " - Probabilidad de movilidad: %.2f\n", PROB_MOV);
        fprintf(resumen, " - Porcentaje de transmisión: %.2f\n", PORC_TRANSMISION);
        fprintf(resumen, " - Beta (tasa infección): %.2f\n", beta);
        fprintf(resumen, " - Gamma (tasa recuperación): %.2f\n\n", gamma);
        
        fprintf(resumen, "Métricas por región:\n");
        fprintf(resumen, "Region | Pico Infectados | Día Pico | Primer Infección | Tiempo Total (s) | Tiempo Com (s)\n");
        fprintf(resumen, "------ | --------------- | -------- | ---------------- | ---------------- | -------------\n");
        
        for (int i = 0; i < size; i++) {
            fprintf(resumen, "%6d | %15.2f | %8d | %16d | %16.6f | %12.6f\n", 
                   i, 
                   todas_metricas[i*3],
                   (int)todas_metricas[i*3+1],
                   (int)todas_metricas[i*3+2],
                   tiempos[i],
                   comm_times[i]);
        }
        
        fprintf(resumen, "\nNOTA: Para calcular speedup, ejecutar por separado con 1 y %d procesos\n", size);
        
        fclose(resumen);
        printf("Resultados guardados en:\n- resultados_global.csv\n- resumen_epidemiologico.txt\n");
        
        free(todas_metricas);
        free(tiempos);
        free(comm_times);
    }

    MPI_Type_free(&tipo_registro);
    MPI_Finalize();
    return 0;
}