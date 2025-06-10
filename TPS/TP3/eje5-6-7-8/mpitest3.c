/*
 * MPI test program.
 *
 * Elias Rudberg, April 2011
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>

#include "sortutils.h"

const int TASK_SEND_DATA = 1;
const int TASK_RECEIVE_AND_MERGE = 2;

static ui8pix* allocateList(int n) {
  ui8pix* ptr = (ui8pix*)malloc(n*sizeof(ui8pix));
  if(!ptr) {
    printf("Error allocating memory; malloc returned NULL. Terminating program.\n");
    exit(-1);
  }
  return ptr;
}

int main(int argc, char *argv[])
{
  /* Initialize MPI. */
  MPI_Init((void *)0, (void *)0);

  /* Get own rank. */
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Get total number of MPI processes. */
  int nProcsTot;
  MPI_Comm_size(MPI_COMM_WORLD, &nProcsTot);

  /* Check that number of MPI processes is a power of 2. */
  int test = nProcsTot;
  while(test > 1) {
    if(test % 2 != 0) {
      printf("Error: number of MPI processes for this program must be a power of 2.\n");
      exit(-1);
    }
    test /= 2;
  }

  /* Print init message if this is rank 0. */
  if (rank == 0) printf("Starting program. This is printf output from rank 0. Total number of processes: %d.\n", nProcsTot);

  /* Create a file named after rank, so that each process can write to its own output file. */
  char fileName[25];
  sprintf(fileName, "output_from_rank_%03d.txt", rank);
  FILE* f = fopen(fileName, "wt");

  /* Get so-called "processor name". */
  char name[MPI_MAX_PROCESSOR_NAME];
  int nameLen;
  MPI_Get_processor_name(name, &nameLen);

  /* Write info about rank and "processor name" to output file. */
  fprintf(f, "This is MPI process with rank %3d running on node '%s'\n", rank, name);
  fflush(f);

  if(rank == 0) {
    /* This is rank 0. */
    /* Create list of random numbers. */
    int N = 10;
    if(argc > 1)
      N = atoi(argv[1]);
    double startTimeSetupList = get_wall_seconds();
    ui8pix* list = allocateList(N);
    for(int i = 0; i < N; i++)
      list[i] = (ui8pix)(rand() % 256);
#if 0
    printf("Original list:\n");
    for(int i = 0; i < N; i++)
      printf("list[%2d] = %d\n", i, (int)list[i]);
#endif
    printf("Setting up list of %d random numbers took %g wall seconds.\n", N, get_wall_seconds()-startTimeSetupList);
    printf("N = %d  ==>  size of list in memory: %g GB.\n", N, (double)N*sizeof(ui8pix)/1000000000);
    /* Check if there is only one process; in that case no communication is needed. */
    if(nProcsTot == 1) {
      printf("Only one process, calling serial sortList routine directly.\n");
      double startTime = get_wall_seconds();
      sortList(N, list);
      printf("sorting list took %g wall seconds.\n", get_wall_seconds()-startTime);
    }
    else {
      printf("Now starting parallel sort algorithm, dividing work among %d MPI processes.\n", nProcsTot);
      double startTime1 = get_wall_seconds();
      /* Divide list into nProcsTot sub-lists and send each sub-list to a worker. */
      int listLengths[nProcsTot];
      int listStartIndexes[nProcsTot];
      int sum = 0;
      for(int i = 0; i < nProcsTot-1; i++) {
        listLengths[i] = N / nProcsTot;
        listStartIndexes[i] = sum;
        sum += listLengths[i];
      }
      listLengths[nProcsTot-1] = N - sum;
      listStartIndexes[nProcsTot-1] = sum;
      /* OK, now we have defined the sub-lists. */
      /* Now send each sub-list (except first one) to a worker. */
      double startTimeSendData = get_wall_seconds();
      for(int i = 1; i < nProcsTot; i++) {
        /* First send a message with a single int giving the length of the sub-list. */
        int intToSend = listLengths[i];
        MPI_Send(&intToSend, 1, MPI_INTEGER, i, 0, MPI_COMM_WORLD);
        /* Now send another message with the sub-list contents. */
        MPI_Send(&list[listStartIndexes[i]], listLengths[i], MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
      }
      printf("Sending data to workers took %g wall seconds.\n", get_wall_seconds()-startTimeSendData);
      /* OK, now we have sent a sub-list to each worker. Now take care of the first sub-list. */
      ui8pix* subList0 = allocateList(listLengths[0]);
      for(int i = 0; i < listLengths[0]; i++)
        subList0[i] = list[i];
      double startTimeSortSubList = get_wall_seconds();
      sortList(listLengths[0], subList0);
      printf("Sorting local sub-list took %g wall seconds.\n", get_wall_seconds()-startTimeSortSubList);
      printf("Own sub-list sorted, now expecting messages from workers that they have finished sorting their lists.\n");
      /* Now collect messages from workers reporting that they have finished sorting sub-lists. */
      double startTimeWaitForWorkers = get_wall_seconds();
      for(int i = 1; i < nProcsTot; i++) {
        MPI_Status status;
        MPI_Recv(NULL, 0, MPI_INTEGER, i, 0, MPI_COMM_WORLD, &status);
      }
      printf("Receiving notifications from workers took %g wall seconds.\n", get_wall_seconds()-startTimeWaitForWorkers);
      printf("OK, all workers have now reported that they have finished sorting their lists.\n");
      /* OK, now all sub-lists are sorted. */
      /* Now tell workers to join their lists two-by-two. */
      int noOfRanksWithData = nProcsTot;
      ui8pix* currList = allocateList(listLengths[0]);
      for(int i = 0; i < listLengths[0]; i++)
        currList[i] = subList0[i];
      int currListLength = listLengths[0];
      int mergeRoundCounter = 0;
      double startTimeMergePhaseTot = get_wall_seconds();
      while(noOfRanksWithData > 1) {
        double startTimeCurrMergeRound = get_wall_seconds();
        /* The highest rank worker with data should send it to rank 0. */
        int intsToSend[2] = {TASK_SEND_DATA, 0};
        MPI_Send(intsToSend, 2, MPI_INTEGER, noOfRanksWithData-1, 0, MPI_COMM_WORLD);
        /* Send messages to other ranks in donor-acceptor pairs. */
        for(int i = 1; i < noOfRanksWithData/2; i++) {
          int intsToSend1[2] = {TASK_SEND_DATA, i};
          MPI_Send(intsToSend1, 2, MPI_INTEGER, noOfRanksWithData-1-i, 0, MPI_COMM_WORLD);
          int intsToSend2[2] = {TASK_RECEIVE_AND_MERGE, noOfRanksWithData-1-i};
          MPI_Send(intsToSend2, 2, MPI_INTEGER, i, 0, MPI_COMM_WORLD);
        }
        /* Now expect to receive data from rank noOfRanksWithData-1. */
        /* First receive one int giving the list length. */
        int lengthToReceive;
        MPI_Status status;
        MPI_Recv(&lengthToReceive, 1, MPI_INTEGER, noOfRanksWithData-1, 0, MPI_COMM_WORLD, &status);
        ui8pix* receivedList = allocateList(lengthToReceive);
        MPI_Recv(receivedList, lengthToReceive, MPI_UNSIGNED_CHAR, noOfRanksWithData-1, 0, MPI_COMM_WORLD, &status);
        /* OK, now we have received sub-list from worker. Now merge. */
        int newLength = currListLength+lengthToReceive;
        ui8pix* newList = allocateList(newLength);
        mergeLists(newList, currList, currListLength, receivedList, lengthToReceive);
        free(currList);
        currList = newList;
        currListLength = newLength;
        noOfRanksWithData /= 2;
        mergeRoundCounter++;
        printf("Merge round %d took %g wall seconds, noOfRanksWithData = %d.\n", 
               mergeRoundCounter, get_wall_seconds()-startTimeCurrMergeRound, noOfRanksWithData);
      }
      printf("Total time for merge phase: %g wall seconds.\n", get_wall_seconds()-startTimeMergePhaseTot);
      /* Now currList should hold the final sorted list. Check that length is correct. */
      if(currListLength != N) {
        printf("Error: length of final list is wrong.\n");
        exit(-1);
      }
      ui8pix* resultList = allocateList(N);
      for(int i = 0; i < N; i++)
        resultList[i] = currList[i];
      /* Done! Now resultList should contain the sorted list. */
      double timeTakenParallel = get_wall_seconds()-startTime1;
      printf("Parallel sort done! Took %g wall seconds.\n", timeTakenParallel);
#if 0
    printf("Result list:\n");
    for(int i = 0; i < N; i++)
      printf("resultList[%2d] = %d\n", i, (int)resultList[i]);
#endif
      double startTimeVerify = get_wall_seconds();
      /* Check that result list is really sorted. */
      for(int i = 0; i < N-1; i++) {
        if(resultList[i] > resultList[i+1]) {
          printf("Error! Result list is not sorted.\n");
          exit(-1);
        }
      }
      printf("Now checking against original list to verify that result is correct.\n");
      int nToCheck = 10;
      for(int i = 0; i < nToCheck; i++) {
        /* Draw a random index to check. */
        int ii = (int)((double)rand()*N / RAND_MAX);
        if(ii < 0) ii = 0;
        if(ii > N-1) ii = N-1;
        ui8pix numberToLookFor = list[ii];
        int found = 0;
        for(int k = 0; k < N; k++) {
          if(resultList[k] == numberToLookFor)
            found = 1;
        }
        if(!found) {
          printf("Error in sort! Result list does not contain all original elements.\n");
          exit(-1);
        }
      }
      printf("Sort checked OK, verifying result took %g wall seconds.\n", get_wall_seconds()-startTimeVerify);
#if 0
      printf("Now calling serial sort to verify result.\n");
      /* To check result, compare with result of serial sort. */
      double startTimeSerial = get_wall_seconds();
      sortList(N, list);
      double timeTakenSerial = get_wall_seconds()-startTimeSerial;
      printf("Serial sort took %g wall seconds.\n", timeTakenSerial);
      printf("timeTakenSerial / timeTakenParallel = %g\n", timeTakenSerial / timeTakenParallel);
      for(int i = 0; i < N; i++) {
        if(resultList[i] != list[i]) {
          printf("Error: result does not match result from serial sort.\n");
          exit(-1);
        }
      }
      printf("Sort result checked OK.\n");
#endif
    }
  }
  else {
    /* This is a worker. */
    /* Receive message from rank 0. */
    int n;
    MPI_Status status;
    fprintf(f, "Calling MPI_Recv to receive n.\n"); fflush(f);
    MPI_Recv(&n, 1, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, &status);
    fprintf(f, "Received integer: %d\n", n); fflush(f);
    /* OK, now we have n. Allocate buffer. */
    ui8pix* list = allocateList(n);
    fprintf(f, "Calling MPI_Recv to receive list.\n"); fflush(f);
    MPI_Recv(list, n, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, &status);
    fprintf(f, "Received list.\n"); fflush(f);
    /* Now sort list. */
    double startTime = get_wall_seconds();
    sortList(n, list);
    fprintf(f, "sorting list took %g wall seconds.\n", get_wall_seconds()-startTime);
    /* Send empty message back to rank 0 to indicate that the work is done. */
    fprintf(f, "Calling MPI_Send.\n"); fflush(f);
    MPI_Send(NULL, 0, MPI_INTEGER, 0, 0, MPI_COMM_WORLD);
    fprintf(f, "MPI_Send done.\n"); fflush(f);
    while(1) {
      fprintf(f, "Expecting message with instructions from rank 0.\n"); fflush(f);
      int intsToReceive[2] = {0, 0};
      MPI_Recv(intsToReceive, 2, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, &status);
      int whatToDo = intsToReceive[0];
      int rankToCommunicateWith = intsToReceive[1];
      if(whatToDo == TASK_SEND_DATA) {
        /* OK, we should send data. First send a single int giving the length. */
        MPI_Send(&n, 1, MPI_INTEGER, rankToCommunicateWith, 0, MPI_COMM_WORLD);
        MPI_Send(list, n, MPI_UNSIGNED_CHAR, rankToCommunicateWith, 0, MPI_COMM_WORLD);
        /* OK, data sent. Nothing more to do, so we break out of while loop. */
        break;
      }
      else if (whatToDo == TASK_RECEIVE_AND_MERGE) {
        /* First receive one int giving the list length. */
        int lengthToReceive;
        MPI_Recv(&lengthToReceive, 1, MPI_INTEGER, rankToCommunicateWith, 0, MPI_COMM_WORLD, &status);
        ui8pix* receivedList = allocateList(lengthToReceive);
        MPI_Recv(receivedList, lengthToReceive, MPI_UNSIGNED_CHAR, rankToCommunicateWith, 0, MPI_COMM_WORLD, &status);
        /* OK, now we have received sub-list from other worker. Now merge. */
        int newLength = n+lengthToReceive;
        ui8pix* newList = allocateList(newLength);
        mergeLists(newList, list, n, receivedList, lengthToReceive);
        free(list);
        list = newList;
        n = newLength;
      }
      else {
        printf("Error: bad instructions received.\n");
        exit(-1);
      }
    }
  }

  /* Close output file. */
  fclose(f);

  /* Finalize MPI. */
  MPI_Finalize();

  exit(0);
}
