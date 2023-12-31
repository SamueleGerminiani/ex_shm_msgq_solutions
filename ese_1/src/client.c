#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "errExit.h"
#include "order.h"

// The readInt reads a integer value from an array of chars
// It checks that only a number n is provided as an input parameter,
// and that n is greater than 0
int readInt(const char *s) {
  char *endptr = NULL;

  errno = 0;
  long int res = strtol(s, &endptr, 10);

  if (errno != 0 || *endptr != '\n' || res < 0) {
    printf("invalid input argument\n");
    exit(1);
  }

  return res;
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

  // get the message queue identifier
  int msqid = msgget(msgKey, S_IRUSR | S_IWUSR);
  if (msqid == -1) errExit("msgget failed");

  char buffer[10];

  // crea an order data struct
  struct order order;

  // by default, the order has type 1
  order.mtype = 1;

  // read the code of the client's order
  printf("Insert order code: ");
  fgets(buffer, sizeof(buffer), stdin);
  order.code = readInt(buffer);

  // read a description of the order
  printf("Insert a description: ");
  fgets(order.description, sizeof(order.description), stdin);

  // read a quantity
  printf("Insert quantity: ");
  fgets(buffer, sizeof(buffer), stdin);
  order.quantity = readInt(buffer);

  // read an e-mail
  printf("Insert an e-mail: ");
  fgets(order.email, sizeof(order.email), stdin);

  printf("Sending the order...\n");
  /* Warning: the POSIX convention requires to remove the type field (long) from
   the message. The type field is not lost, instead, it is used by the kernel to
   store the type of the message. The 'type' field is simply not part of the
   message content; therefore, we should not include it in the size of the
   message.
   */
  size_t mSize = sizeof(struct order) - sizeof(long);
  // send the order to the server through the message queue
  if (msgsnd(msqid, &order, mSize, 0) == -1) errExit("msgsnd failed");

  printf("Done\n");
  return 0;
}
