#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>

#include "errExit.h"
#include "order.h"

// the message queue identifier
int msqid = -1;

void sigIntHandler(int sig) {
  // do we have a valid message queue identifier
  if (msqid > 0) {
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
      errExit("msgctl failed");
    else
      printf("<Server> message queue removed successfully\n");
  }

  // terminate the server process
  exit(0);
}

int main(int argc, char *argv[]) {
  // check command line input arguments
  if (argc != 2) {
    printf("Usage: %s message_queue_key\n", argv[0]);
    exit(1);
  }

  // read the message queue key defined by user
  int msgKey = atoi(argv[1]);
  if (msgKey <= 0) {
    printf("The message queue key must be greater than zero!\n");
    exit(1);
  }

  // set of signals (N.B. it is not initialized!)
  sigset_t mySet;
  // initialize mySet to contain all signals
  sigfillset(&mySet);
  // remove SIGINT from mySet
  sigdelset(&mySet, SIGINT);
  // blocking all signals but SIGINT
  sigprocmask(SIG_SETMASK, &mySet, NULL);

  // set the function sigHandler as handler for the signal SIGINT
  if (signal(SIGINT, sigIntHandler) == SIG_ERR)
    errExit("change signal handler failed");

  printf("<Server> Making MSG queue...\n");
  // get the message queue, or create a new one if it does not exist
  msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);
  if (msqid == -1) errExit("msgget failed");

  // check functionality
  printf("<Server> sleep...\n");
  sleep(30);
  // the process sleeps for 30 seconds. Try to send some orders
  // and check that prime users' orders are always read before
  // normal users' ones

  struct order order;
  // endless loop
  while (1) {
    // type is set equal to -2. Thus, first all messages with type 1 are read.
    // When no message with type 1 is in the queue, then messages with type
    // 2 are read from the queue.
    size_t mSize = sizeof(struct order) - sizeof(long);
    // read a message from the message queue.
    if (msgrcv(msqid, &order, mSize, -2, 0) == -1) errExit("msgget failed");

    // print the order on standard output
    printOrder(&order);
  }

  return 0;
}
