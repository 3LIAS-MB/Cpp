#include <mpi.h>
#include <stdio.h>

#define  MASTER	0

int main (int argc, char *argv[]){
	int  rankid, size, length;
	char hostname[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);
	MPI_Get_processor_name(hostname, &length);
	if (rankid == MASTER){
		printf ("\nNodo Master dice: Hello!!!! Cantidad de Procesos %d  Server %s\n\n", size, hostname);	
	}else{
		printf ("Nodo Esclavo dice Hello!!!! Proceso %d del host %s\n", rankid, hostname);	
	}
	MPI_Finalize();
}