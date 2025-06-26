#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <math.h>
#include <stdbool.h>

#define MAX_VECINOS 10
#define MAX_LINEA 256
#define MAX_REGIONES 100

typedef struct {
    double S, I, R, N;  // Población como double
    int id;
    int num_vecinos;
    int vecinos[MAX_VECINOS];
} Region;

typedef struct {
    int dia;
    double S, I, R;
} Registro;

// Estructura para parámetros configurables
typedef struct {
    double beta;
    double gamma;
    int dias;
    double prob_mov;
    unsigned int semilla;
    bool modo_secuencial;
    char archivo_salida[100];
    char archivo_entrada[100];
} Configuracion;

void leer_configuracion(int argc, char* argv[], int rank, Configuracion* config) {
    // Valores predeterminados
    config->beta = 2.0;
    config->gamma = 1.0;
    config->dias = 30;
    config->prob_mov = 0.1;
    config->semilla = (unsigned int)time(NULL);
    config->modo_secuencial = false;
    strcpy(config->archivo_salida, "resultados_sir.csv");
    
    if (argc < 2) {
        if (rank == 0) {
            printf("Uso: %s archivo_entrada [beta gamma dias prob_mov semilla modo_salida archivo_salida]\n", argv[0]);
            printf("Ejemplo: %s datos.txt 2.5 0.8 60 0.15 1234 paralelo resultados.csv\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    strcpy(config->archivo_entrada, argv[1]);
    
    // Parámetros opcionales
    if (argc > 2) config->beta = atof(argv[2]);
    if (argc > 3) config->gamma = atof(argv[3]);
    if (argc > 4) config->dias = atoi(argv[4]);
    if (argc > 5) config->prob_mov = atof(argv[5]);
    if (argc > 6) config->semilla = (unsigned int)atoi(argv[6]);
    if (argc > 7) config->modo_secuencial = (strcmp(argv[7], "secuencial") == 0);
    if (argc > 8) strcpy(config->archivo_salida, argv[8]);
}

void leer_datos_region(const char* archivo, int rank, Region* reg) {
    FILE* f = fopen(archivo, "r");
    if (!f) {
        fprintf(stderr, "Error: No se pudo abrir %s\n", archivo);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char linea[MAX_LINEA];
    int actual = -1;
    bool encontrado = false;

    while (fgets(linea, sizeof(linea), f) {
        if (linea[0] == '#' || strlen(linea) < 3) continue;
        actual++;
        if (actual == rank) {
            char* token = strtok(linea, " ");
            reg->N = atof(token);  // Leer como double
            reg->S = atof(strtok(NULL, " "));
            reg->I = atof(strtok(NULL, " "));
            reg->R = atof(strtok(NULL, " "));
            reg->id = actual;

            reg->num_vecinos = 0;
            while ((token = strtok(NULL, " \n")) != NULL && reg->num_vecinos < MAX_VECINOS) {
                reg->vecinos[reg->num_vecinos++] = atoi(token);
            }
            encontrado = true;
            break;
        }
    }

    fclose(f);

    if (!encontrado) {
        fprintf(stderr, "Error: Region %d no encontrada en %s\n", rank, archivo);
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

void simular_region(Region* region, int dia, double beta, double gamma, double prob_mov) {
    // Actualización del modelo SIR
    double newS = region->S - beta * region->S * region->I / region->N;
    double newI = region->I + beta * region->S * region->I / region->N - gamma * region->I;
    double newR = region->R + gamma * region->I;

    // Evitar valores negativos
    region->S = fmax(newS, 0.0);
    region->I = fmax(newI, 0.0);
    region->R = fmax(newR, 0.0);
    
    // Actualizar población total
    region->N = region->S + region->I + region->R;
}

void simular_secuencial(Configuracion config) {
    FILE* entrada = fopen(config.archivo_entrada, "r");
    if (!entrada) {
        fprintf(stderr, "Error: No se pudo abrir %s\n", config.archivo_entrada);
        exit(1);
    }

    // Contar regiones
    char linea[MAX_LINEA];
    int num_regiones = 0;
    while (fgets(linea, sizeof(linea), entrada)) {
        if (linea[0] != '#' && strlen(linea) > 2) num_regiones++;
    }
    rewind(entrada);

    // Leer todas las regiones
    Region* regiones = malloc(num_regiones * sizeof(Region));
    for (int i = 0; i < num_regiones; i++) {
        fgets(linea, sizeof(linea), entrada);
        while (linea[0] == '#' || strlen(linea) < 3) {
            fgets(linea, sizeof(linea), entrada);
        }
        
        char* token = strtok(linea, " ");
        regiones[i].N = atof(token);
        regiones[i].S = atof(strtok(NULL, " "));
        regiones[i].I = atof(strtok(NULL, " "));
        regiones[i].R = atof(strtok(NULL, " "));
        regiones[i].id = i;
        
        regiones[i].num_vecinos = 0;
        while ((token = strtok(NULL, " \n")) != NULL && regiones[i].num_vecinos < MAX_VECINOS) {
            regiones[i].vecinos[regiones[i].num_vecinos++] = atoi(token);
        }
    }
    fclose(entrada);

    // Preparar registros
    Registro** historial = malloc(num_regiones * sizeof(Registro*));
    for (int i = 0; i < num_regiones; i++) {
        historial[i] = malloc((config.dias + 1) * sizeof(Registro));
    }

    // Semilla aleatoria
    srand(config.semilla);
    double inicio = MPI_Wtime();

    // Simulación
    for (int dia = 0; dia <= config.dias; dia++) {
        // Registrar estado actual
        for (int i = 0; i < num_regiones; i++) {
            historial[i][dia].dia = dia;
            historial[i][dia].S = regiones[i].S;
            historial[i][dia].I = regiones[i].I;
            historial[i][dia].R = regiones[i].R;
        }

        // Aplicar modelo SIR
        for (int i = 0; i < num_regiones; i++) {
            simular_region(&regiones[i], dia, config.beta, config.gamma, config.prob_mov);
        }

        // Migración entre regiones
        for (int i = 0; i < num_regiones; i++) {
            for (int v = 0; v < regiones[i].num_vecinos; v++) {
                int vecino = regiones[i].vecinos[v];
                double cantidad = config.prob_mov * regiones[i].I / regiones[i].num_vecinos;
                cantidad = fmin(cantidad, regiones[i].I);
                
                regiones[i].I -= cantidad;
                regiones[vecino].I += cantidad;
            }
        }

        // Actualizar poblaciones después de migración
        for (int i = 0; i < num_regiones; i++) {
            regiones[i].N = regiones[i].S + regiones[i].I + regiones[i].R;
        }
    }

    double tiempo = MPI_Wtime() - inicio;

    // Escribir resultados
    FILE* salida = fopen(config.archivo_salida, "w");
    fprintf(salida, "region,dia,susceptible,infectado,recuperado\n");
    for (int d = 0; d <= config.dias; d++) {
        for (int i = 0; i < num_regiones; i++) {
            fprintf(salida, "%d,%d,%.4f,%.4f,%.4f\n",
                    i, historial[i][d].dia,
                    historial[i][d].S,
                    historial[i][d].I,
                    historial[i][d].R);
        }
    }
    fclose(salida);

    // Reporte de ejecución
    printf("\n=== SIMULACION COMPLETADA ===\n");
    printf("Modo: secuencial\n");
    printf("Regiones: %d\n", num_regiones);
    printf("Dias simulados: %d\n", config.dias);
    printf("Tiempo ejecucion: %.4f segundos\n", tiempo);
    printf("Archivo resultados: %s\n", config.archivo_salida);
    printf("Parametros: beta=%.2f, gamma=%.2f, prob_mov=%.2f\n",
           config.beta, config.gamma, config.prob_mov);
    
    // Liberar memoria
    for (int i = 0; i < num_regiones; i++) free(historial[i]);
    free(historial);
    free(regiones);
}

int main(int argc, char* argv[]) {
    // Inicialización MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Configuración
    Configuracion config;
    leer_configuracion(argc, argv, rank, &config);
    
    // Ejecutar modo secuencial si se solicitó
    if (config.modo_secuencial) {
        if (rank == 0) simular_secuencial(config);
        MPI_Finalize();
        return 0;
    }

    // Leer datos de la región
    Region region;
    leer_datos_region(config.archivo_entrada, rank, &region);

    // Inicialización
    srand(config.semilla + rank);
    MPI_Datatype tipo_registro = crear_tipo_registro();
    Registro* registros = malloc((config.dias + 1) * sizeof(Registro));
    double inicio = MPI_Wtime();

    // Buffers para comunicación
    double* enviar = malloc(region.num_vecinos * sizeof(double));
    double* recibir = malloc(region.num_vecinos * sizeof(double));
    MPI_Request* req_send = malloc(region.num_vecinos * sizeof(MPI_Request));
    MPI_Request* req_recv = malloc(region.num_vecinos * sizeof(MPI_Request));

    // Simulación
    for (int dia = 0; dia <= config.dias; dia++) {
        // Registrar estado actual
        registros[dia].dia = dia;
        registros[dia].S = region.S;
        registros[dia].I = region.I;
        registros[dia].R = region.R;

        // Aplicar modelo SIR
        simular_region(&region, dia, config.beta, config.gamma, config.prob_mov);

        // Preparar comunicación con vecinos
        for (int i = 0; i < region.num_vecinos; i++) {
            // Calcular cantidad a enviar (fracción de infectados)
            double cantidad = config.prob_mov * region.I / region.num_vecinos;
            cantidad = fmin(cantidad, region.I);  // No enviar más de lo disponible
            
            enviar[i] = cantidad;
            region.I -= cantidad;
            
            MPI_Irecv(&recibir[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_recv[i]);
        }

        // Enviar a vecinos
        for (int i = 0; i < region.num_vecinos; i++) {
            MPI_Isend(&enviar[i], 1, MPI_DOUBLE, region.vecinos[i], 0, MPI_COMM_WORLD, &req_send[i]);
        }

        // Recibir de vecinos
        MPI_Waitall(region.num_vecinos, req_recv, MPI_STATUSES_IGNORE);
        for (int i = 0; i < region.num_vecinos; i++) {
            region.I += recibir[i];
        }
        MPI_Waitall(region.num_vecinos, req_send, MPI_STATUSES_IGNORE);

        // Actualizar población total después de migración
        region.N = region.S + region.I + region.R;
    }

    double tiempo = MPI_Wtime() - inicio;

    // Recolección de resultados
    if (rank == 0) {
        FILE* f = fopen(config.archivo_salida, "w");
        fprintf(f, "region,dia,susceptible,infectado,recuperado\n");
        
        // Escribir datos del proceso 0
        for (int d = 0; d <= config.dias; d++) {
            fprintf(f, "%d,%d,%.4f,%.4f,%.4f\n",
                    rank, registros[d].dia,
                    registros[d].S,
                    registros[d].I,
                    registros[d].R);
        }

        // Recibir datos de otros procesos
        for (int src = 1; src < size; src++) {
            Registro* otros = malloc((config.dias + 1) * sizeof(Registro));
            MPI_Recv(otros, config.dias + 1, tipo_registro, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            for (int d = 0; d <= config.dias; d++) {
                fprintf(f, "%d,%d,%.4f,%.4f,%.4f\n",
                        src, otros[d].dia,
                        otros[d].S,
                        otros[d].I,
                        otros[d].R);
            }
            free(otros);
        }
        fclose(f);
        
        // Reporte de ejecución
        printf("\n=== SIMULACION COMPLETADA ===\n");
        printf("Modo: paralelo (MPI)\n");
        printf("Procesos: %d\n", size);
        printf("Regiones simuladas: %d\n", size);
        printf("Dias simulados: %d\n", config.dias);
        printf("Tiempo ejecucion: %.4f segundos\n", tiempo);
        printf("Archivo resultados: %s\n", config.archivo_salida);
        printf("Parametros: beta=%.2f, gamma=%.2f, prob_mov=%.2f, semilla=%u\n",
               config.beta, config.gamma, config.prob_mov, config.semilla);
    } else {
        MPI_Send(registros, config.dias + 1, tipo_registro, 0, 1, MPI_COMM_WORLD);
    }

    // Liberar recursos
    free(registros);
    free(enviar);
    free(recibir);
    free(req_send);
    free(req_recv);
    MPI_Type_free(&tipo_registro);
    MPI_Finalize();
    return 0;
}
