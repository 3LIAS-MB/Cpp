#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <math.h>

#define MAX_VECINOS 10
#define MAX_LINEA 256
#define DIAS 120            // 120 días de simulación
#define PROB_MOV 0.50       // 50% de probabilidad de movilidad
#define PORC_TRANSMISION 0.02 // 2% de infectados se mueven

typedef struct {
    double S, I, R;
    int N;
    int num_vecinos;
    int vecinos[MAX_VECINOS];
    double peak_infection;  // Máximo de infectados
    int peak_day;           // Día del pico máximo
    int first_infection_day; // Primer día con infectados
} Region;

typedef struct {
    int dia;
    double S, I, R;
} Registro;

void leer_datos_region(const char* archivo, int rank, Region* reg) {
    FILE* f = fopen(archivo, "r");
    if (!f) {
        fprintf(stderr, "No se pudo abrir el archivo %s\n", archivo);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char linea[MAX_LINEA];
    int actual = -1;
    int encontrado = 0;

    while (fgets(linea, sizeof(linea), f)) {
        if (linea[0] == '#' || strlen(linea) < 4) continue;
        actual++;
        if (actual == rank) {
            char* token = strtok(linea, " ");
            reg->N = atoi(token);
            reg->S = atof(strtok(NULL, " "));
            reg->I = atof(strtok(NULL, " "));
            reg->R = atof(strtok(NULL, " "));

            // Inicializar métricas de seguimiento
            reg->peak_infection = reg->I;
            reg->peak_day = 0;
            reg->first_infection_day = (reg->I > 0) ? 0 : -1;

            reg->num_vecinos = 0;
            while ((token = strtok(NULL, " ")) != NULL && reg->num_vecinos < MAX_VECINOS) {
                reg->vecinos[reg->num_vecinos++] = atoi(token);
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
    leer_datos_region(argv[1], rank, &region);

    // Semilla única por región
    srand((unsigned)time(NULL) + rank * 10007);

    MPI_Datatype tipo_registro = crear_tipo_registro();
    Registro registros[DIAS + 1];

    // Parámetros epidemiológicos (actualizados para coincidir con el ejemplo)
    double beta = 0.80, gamma = 0.10;  // R0 = 8

    double enviar[MAX_VECINOS] = {0};
    double recibir[MAX_VECINOS] = {0};
    MPI_Request req_send[MAX_VECINOS];
    MPI_Request req_recv[MAX_VECINOS];
    
    // Para medir overhead de comunicación
    double total_comm_time = 0.0;
    double comm_start, comm_end;

    MPI_Barrier(MPI_COMM_WORLD);
    double t_inicio = MPI_Wtime();
    
    for (int dia = 0; dia <= DIAS; dia++) {
        // Registrar estado actual
        registros[dia].dia = dia;
        registros[dia].S = region.S;
        registros[dia].I = region.I;
        registros[dia].R = region.R;

        // Actualizar métricas de pico de infección
        if (region.I > region.peak_infection) {
            region.peak_infection = region.I;
            region.peak_day = dia;
        }

        // Modelo SIR
        double nuevos_infectados = beta * region.S * region.I / region.N;
        double nuevos_recuperados = gamma * region.I;
        
        // Actualizar estados
        region.S -= nuevos_infectados;
        region.I += nuevos_infectados - nuevos_recuperados;
        region.R += nuevos_recuperados;

        // Garantizar consistencia numérica
        double total = region.S + region.I + region.R;
        if (fabs(total - region.N) > 1e-5) {
            double factor = region.N / total;
            region.S *= factor;
            region.I *= factor;
            region.R *= factor;
        }

        // COMUNICACIÓN ENTRE REGIONES =====================
        comm_start = MPI_Wtime();
        
        double total_enviado = 0.0;
        // Preparar envíos a vecinos
        for (int i = 0; i < region.num_vecinos; i++) {
            if ((double)rand() / RAND_MAX < PROB_MOV && region.I > 1.0) {
                enviar[i] = region.I * PORC_TRANSMISION;
                if (enviar[i] + total_enviado > region.I) {
                    enviar[i] = region.I - total_enviado;
                }
                total_enviado += enviar[i];
            } else {
                enviar[i] = 0.0;
            }
        }
        region.I -= total_enviado;

        // Comunicación no bloqueante
        for (int i = 0; i < region.num_vecinos; i++) {
            MPI_Irecv(&recibir[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_recv[i]);
        }

        for (int i = 0; i < region.num_vecinos; i++) {
            MPI_Isend(&enviar[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_send[i]);
        }

        // Procesar llegada de infectados
        MPI_Waitall(region.num_vecinos, req_recv, MPI_STATUSES_IGNORE);
        for (int i = 0; i < region.num_vecinos; i++) {
            region.I += recibir[i];
            // Registrar primera infección entrante
            if (recibir[i] > 0 && region.first_infection_day == -1) {
                region.first_infection_day = dia;
            }
        }
        MPI_Waitall(region.num_vecinos, req_send, MPI_STATUSES_IGNORE);
        
        comm_end = MPI_Wtime();
        total_comm_time += (comm_end - comm_start);
        // FIN COMUNICACIÓN
    }

    double t_fin = MPI_Wtime();
    double tiempo_total = t_fin - t_inicio;

    // Recolección de resultados epidemiológicos
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

        // Imprimir tiempos de ejecución
        printf("Tiempo total de ejecución: %.6f segundos\n", tiempo_total);
        printf("Tiempo total en comunicación: %.6f segundos\n", total_comm_time);
        printf("Porcentaje de tiempo en comunicación: %.2f%%\n", (total_comm_time/tiempo_total)*100);
    } else {
        MPI_Send(registros, DIAS + 1, tipo_registro, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Type_free(&tipo_registro);
    MPI_Finalize();
    return 0;
}