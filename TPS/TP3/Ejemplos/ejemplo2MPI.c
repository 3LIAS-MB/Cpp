#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define  MASTER	 0

int main (int argc, char *argv[]){
	int  rankid, size, length, origen, destino, tag = 1;
	// MPI_MAX_PROCESSOR_NAME es la longitud máxima del nombre del procesador.
	
	char hostname[MPI_MAX_PROCESSOR_NAME];
	char mensaje[100];
	// MPI_Status almacena el estado de la operación de recepción.
	MPI_Status status;
	// MPI_Init(&argc, &argv) inicializa el entorno MPI.
	MPI_Init(&argc, &argv);
	
	// MPI_Comm_rank() obtiene el identificador (rank) del proceso actual.
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);
	// MPI_Comm_size() obtiene el número total de procesos.	
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (rankid != MASTER){
		// MPI_Get_processor_name() obtiene el nombre del procesador.
		MPI_Get_processor_name(hostname, &length);
		sprintf(mensaje, "Hello!!! desde el nodo %s", hostname);
		destino = 0;
		// MPI_Send() envía un mensaje al proceso destino.
		MPI_Send(mensaje, strlen(mensaje) + 1, MPI_CHAR, destino, tag, MPI_COMM_WORLD);
	}else{
		printf ("Nodo Master dice: Hello!!!! Cantidad de Procesos %d\n", size);
		for (origen = 1; origen < size; origen++){
			// MPI_Recv() recibe un mensaje del proceso origen.
			MPI_Recv(mensaje, 100, MPI_CHAR, origen, tag, MPI_COMM_WORLD, &status);
			printf("%s\n", mensaje);
		}
	}
	// MPI_Finalize() finaliza el entorno MPI. 
	MPI_Finalize();
}