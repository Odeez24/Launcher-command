#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "file_sync.h"

// run: fonction de lancement du thread
void *run(void *arg);

int main(void) {
  int errnum;
  pid_t d;

  pthread_t th;
  if ((errnum = pthread_create(&th, NULL, run, NULL)) != 0) {
      fprintf(stderr, "pthread_create: %s\n", strerror(errnum));
      exit(EXIT_FAILURE);
  }

}
