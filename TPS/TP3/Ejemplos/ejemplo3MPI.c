#include <mpi.h>
#include <stdio.h>
#define  MASTER	 0

int main (int argc, char *argv[]){
	int  rankid, size, i, j=0, tag = 1, suma=0;
	int pares[100];
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (rankid == MASTER){
		printf ("Cargo pares en el proceso %d\n", rankid);
		for (i = 1; i <= 100; i++){
			if (i%2 == 0){
				pares[j] = i;
				++j;
			}
		}
		MPI_Send(pares, 50, MPI_INT, 1, tag, MPI_COMM_WORLD);
		MPI_Send(pares, 50, MPI_INT, 2, tag, MPI_COMM_WORLD);
//	 	MPI_Recv(&suma, 1, MPI_INT, 2, tag, MPI_COMM_WORLD, &status);
//		printf ("Total suma pares %d\n", suma);	
	}else if (rankid == 1 ) {
	   	MPI_Recv(pares, 50, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		printf ("Muestro pares en el proceso %d\n", rankid);
		for (i = 0; i < 50; i++){
			printf ("Par %d\n", pares[i]);
		}
	}else if (rankid == 2 ) {
	   	MPI_Recv(pares, 50, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		printf ("Suma de pares en el proceso %d\n", rankid);
		for (i = 0; i < 50; i++){
			suma += pares[i];
		}
//		MPI_Send(&suma, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
		printf ("Total suma pares %d\n", suma);
	}
	MPI_Finalize();
	return(0);
}