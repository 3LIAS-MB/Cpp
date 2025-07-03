// biblioteca principal de MPI que proporciona las funciones
// necesarias para la comunicación entre procesos.
#include <mpi.h>
// Se utiliza para funciones de entrada/salida, como printf.
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Esta función inicializa el entorno de MPI.Es obligatoria y
    // debe ser llamada antes de cualquier otra función de MPI.
    MPI_Init(&argc, &argv);

    int rank;                                                                                           
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        int data = 42;
        MPI_Send(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        int received_data;
        MPI_Status status;

        MPI_Recv(&received_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        // Valores almacenados en `status`
        printf("Origen del mensaje: %d\n", status.MPI_SOURCE); // Debería ser 0
        printf("Etiqueta del mensaje: %d\n", status.MPI_TAG);   // Debería ser 0
    }

    MPI_Finalize();
    return 0;
}