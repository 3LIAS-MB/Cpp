#include <mpi.h>
#include <stdio.h>
 
int main (int argc, char *argv[]){
	int  rankid, size, length;
	char hostname[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);
	MPI_Get_processor_name(hostname, &length);
	printf ("Hello!!!! desde el proceso %d de %d del host %s\n", rankid, size, hostname);
	MPI_Finalize();
}