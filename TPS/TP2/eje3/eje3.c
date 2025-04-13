#include <omp.h>
#include <stdio.h>
int main() {
int TAM = 16;
int i, id, threads;
double vectorY[] = {12,51,11,21,12,13,23,34,100,45,56,11,10,5,17,5};
double vectorX[] = {21,25,31,42,41,23,12,34,23,30,78,99,12,32,1,11};
double vectorZ[TAM];
// omp_set_num_threads(2);
#pragma omp parallel for private(id)
for(i=0; i < TAM; ++i){
vectorZ[i] = vectorX[i] + vectorY[i];
id = omp_get_thread_num();
printf("El thread %d - Procesa el indice: %d\n", id, i);
}
for(i=0; i < TAM; ++i){
printf("VectorZ %f\n", vectorZ[i]);
}
}