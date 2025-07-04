#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h> // Para offsetof

#define MAX_VECINOS 10
#define MAX_LINEA 256
#define DIAS 30
#define PROB_MOV 0.1 // Probabilidad reducida para evitar propagación explosiva

typedef struct {
    double S, I, R;
    int N;
    int num_vecinos;
    int vecinos[MAX_VECINOS];
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
        if (linea[0] == '#' || strlen(linea) < 3) continue;
        actual++;
        if (actual == rank) {
            char* token = strtok(linea, " ");
            reg->N = atoi(token);
            reg->S = atof(strtok(NULL, " "));
            reg->I = atof(strtok(NULL, " "));
            reg->R = atof(strtok(NULL, " "));

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

    srand((unsigned)time(NULL) + rank * 10007);

    MPI_Datatype tipo_registro = crear_tipo_registro();
    Registro registros[DIAS + 1];

    double beta = 2, gamma = 1;

    double enviar[MAX_VECINOS];
    double recibir[MAX_VECINOS];
    MPI_Request req_send[MAX_VECINOS];
    MPI_Request req_recv[MAX_VECINOS];

    MPI_Barrier(MPI_COMM_WORLD); // Sincronización antes de iniciar

    // ⏱ Tiempo de inicio
    double t_inicio = MPI_Wtime();
    
    for (int dia = 0; dia <= DIAS; dia++) {
        registros[dia].dia = dia;
        registros[dia].S = region.S;
        registros[dia].I = region.I;
        registros[dia].R = region.R;

        // Actualización del modelo SIR
        double newS = region.S - beta * region.S * region.I / region.N;
        double newI = region.I + beta * region.S * region.I / region.N - gamma * region.I;
        double newR = region.R + gamma * region.I;

        // Evitar valores negativos
        region.S = (newS < 0) ? 0 : newS;
        region.I = (newI < 0) ? 0 : newI;
        region.R = (newR < 0) ? 0 : newR;

        // Comunicación con vecinos
        for (int i = 0; i < region.num_vecinos; i++) {
            double cantidad_a_enviar = (region.I >= 1.0) ? 1.0 : 0.0;
            if ((double)rand() / RAND_MAX < PROB_MOV && cantidad_a_enviar > 0) {
                enviar[i] = cantidad_a_enviar;
                region.I -= cantidad_a_enviar;
            } else {
                enviar[i] = 0.0;
            }
            MPI_Irecv(&recibir[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_recv[i]);
        }

        for (int i = 0; i < region.num_vecinos; i++) {
            MPI_Isend(&enviar[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_send[i]);
        }

        MPI_Waitall(region.num_vecinos, req_recv, MPI_STATUSES_IGNORE);
        for (int i = 0; i < region.num_vecinos; i++) {
            region.I += recibir[i];
        }

        MPI_Waitall(region.num_vecinos, req_send, MPI_STATUSES_IGNORE);
    }

    // Recolección de resultados
    if (rank == 0) {
        FILE* f = fopen("resultados_global.csv", "w");
        if (!f) {
            fprintf(stderr, "No se pudo abrir el archivo de salida\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(f, "Region,Dia,S,I,R\n");
        for (int d = 0; d <= DIAS; d++) {
            fprintf(f, "%d,%d,%.2f,%.2f,%.2f\n", rank, registros[d].dia, registros[d].S, registros[d].I, registros[d].R);
        }

        for (int src = 1; src < size; src++) {
            Registro otros[DIAS + 1];
            MPI_Recv(otros, DIAS + 1, tipo_registro, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int d = 0; d <= DIAS; d++) {
                fprintf(f, "%d,%d,%.2f,%.2f,%.2f\n", src, otros[d].dia, otros[d].S, otros[d].I, otros[d].R);
            }
        }

        fclose(f);
        printf("Resultados guardados en resultados_global.csv\n");

        // ⏱ Tiempo de fin e impresión
        double t_fin = MPI_Wtime();
        printf("Tiempo total de ejecución: %.4f segundos\n", t_fin - t_inicio);
    } else {
        MPI_Send(registros, DIAS + 1, tipo_registro, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Type_free(&tipo_registro);
    MPI_Finalize();
    return 0;
}
////// ultima modificacion
