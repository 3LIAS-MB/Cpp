/*
 * Sort utilities for MPI test program.
 *
 * Elias Rudberg, April 2011
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "sortutils.h"

double get_wall_seconds() {
  struct timeval tv; gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec / 1000000;
}

void mergeLists(ui8pix* resultList, const ui8pix* list1, int n1, const ui8pix* list2, int n2) {
  int i1 = 0;
  int i2 = 0;
  int k = 0;
  while(i1 < n1 && i2 < n2) {
    if(list1[i1] < list2[i2]) {
      resultList[k++] = list1[i1++];
    }
    else {
      resultList[k++] = list2[i2++];
    }
  }
  while(i1 < n1)
    resultList[k++] = list1[i1++];
  while(i2 < n2)
    resultList[k++] = list2[i2++];
}

#if 1
static int compare_ints(const void* p1, const void* p2) {
  ui8pix* ptr1 = (ui8pix*)p1;
  ui8pix* ptr2 = (ui8pix*)p2;
  if(*ptr1 < *ptr2)
    return -1;
  if(*ptr1 > *ptr2)
    return 1;
  return 0;
}
void sortList(int n, ui8pix* list) {
  qsort(list, n, sizeof(ui8pix), compare_ints);
  // Verify that list is sorted.
  for(int i = 0; i < n-1; i++) {
    if(list[i] > list[i+1]) {
      printf("Error: list not sorted!\n");
      exit(-1);
    }
  }
}
#else
void sortList(int n, int* list) {
  /* Bubble sort. */
  for(int i = 0; i < n-1; i++)
    for(int j = 0; j < n-i-1; j++) {
      if(list[j] > list[j+1]) {
        // swap.
        int tmp = list[j];
        list[j] = list[j+1];
        list[j+1] = tmp;
      }
    }
  // Verify that list is sorted.
  for(int i = 0; i < n-1; i++) {
    if(list[i] > list[i+1]) {
      printf("Error: list not sorted!\n");
      exit(-1);
    }
  }
}
#endif

