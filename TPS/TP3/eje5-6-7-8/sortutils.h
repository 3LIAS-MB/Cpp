/*
 * Sort utilities for MPI test program.
 *
 * Elias Rudberg, April 2011
 */

typedef unsigned char ui8pix;

double get_wall_seconds();

void mergeLists(ui8pix* resultList, const ui8pix* list1, int n1, const ui8pix* list2, int n2);

void sortList(int n, ui8pix* list);

