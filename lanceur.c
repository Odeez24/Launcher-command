#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <file_sync.h>

//--- Marco nom des tubes ------------------------------------------------------

#define TUBE_CL "TUBE_CLIENT_"
#define TUBE_RES "TUBE_RES_CLIENT_"
#define TUBE_ERR "TUBE_ERR_CLIENT_"

//--- Outils pour les threads --------------------------------------------------
/*
 * Les arguments transmis au thread.
 */
struct my_thread_args {
  pid_t client; // Le pid_t du clint pour lequel le thread est crée
};

// run: fonction de lancement du thread
void *run(struct my_thread_args *a);

typedef void *(*start_routine_type)(void *);

//--- Outils pour les signaux --------------------------------------------------
// Variable pour la fin de l'exécution
bool work;

//  mafct: fonction de gestion des signaux
void mafct(int sig);

//--- Main ---------------------------------------------------------------------
int main(void) {
  int errnum;
  work = true;
  if (create_file_sync() == -1) {
    fprintf(stderr, "Error during create_file_sync");
    exit(EXIT_FAILURE);
  }
  int nbth = 0;
  struct sigaction act;
  act.sa_handler = mafct;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, NULL);
  pthread_t th;
  while (work) {
    pid_t p = defiler();
    struct my_thread_args *a = malloc(sizeof(struct my_thread_args));
    if (a == NULL) {
      fprintf(stderr, "Error: malloc");
      exit(EXIT_FAILURE);
    }
    a->client = p;
    if ((errnum
          = pthread_create(&th, NULL, (start_routine_type) run, a)) != 0) {
      fprintf(stderr, "pthread_create: %s\n", strerror(errnum));
      exit(EXIT_FAILURE);
    }
    ++nbth;
  }
  for (int i = 0; i < nbth; ++i) {
    int *ptr;
    if ((errnum = pthread_join(th, (void **) &ptr)) != 0) {
      fprintf(stderr, "pthread_join: %s\n", strerror(errnum));
      exit(EXIT_FAILURE);
    }
    free(ptr);
  }
  if (destroy_file() == -1) {
    fprintf(stderr, "Error during destroy_file");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

void *run(struct my_thread_args *a) {
  int fd;
  char pid[101];
  int pidlen;
  if ((pidlen = snprintf(pid, UCHAR_MAX, "%d", a->client)) < 0
      || pidlen > 100) {
    return NULL;
  }
  pid[100] = '\0';
  char tube_cl[(int) strlen(TUBE_CL) + pidlen];
  strcpy(tube_cl, TUBE_CL);
  strncat(tube_cl, pid, (size_t) pidlen);
  char tube_res[(int) strlen(TUBE_RES) + pidlen];
  strcpy(tube_res, TUBE_RES);
  strncat(tube_res, pid, (size_t) pidlen);
  char tube_err[(int) strlen(TUBE_ERR) + pidlen];
  strcpy(tube_err, TUBE_ERR);
  strncat(tube_err, pid, (size_t) pidlen);
  switch (fork()) {
    case -1:
      return NULL;
    case 0:
      if ((fd = open(tube_cl, O_RDONLY)) == -1) {
        return NULL;
      }
      char c;
      char cmd[40];
      char opt[100];
      int i = 0;
      while ((c = (char) read(fd, &c, sizeof(char))) > 0 && c != '-') {
        if (c == -1) {
          return NULL;
        }
        cmd[i] = c;
        ++i;
      }
      cmd[i] = '\0';
      i = 0;
      while ((c = (char) read(fd, &c, sizeof(char))) > 0) {
        if (c == -1) {
          return NULL;
        }
        opt[i] = c;
        ++i;
      }
      opt[i] = '\0';
      if (close(fd) == -1) {
        return NULL;
      }
      int fd_res;
      int fd_err;
      if ((fd_res = open(tube_res, O_WRONLY) == -1)) {
        return NULL;
      }
      if ((fd_err = open(tube_err, O_WRONLY) == -1)) {
        return NULL;
      }
      if (dup2(fd_res, STDOUT_FILENO) == -1) {
        return NULL;
      }
      if (close(fd_res) == -1) {
        return NULL;
      }
      if (dup2(fd_err, STDERR_FILENO) == -1) {
        return NULL;
      }
      if (close(fd_err) == -1) {
        return NULL;
      }
      execvp(cmd, (char * const *) opt);
      fprintf(stderr, "Error during the execution of the command");
      exit(EXIT_FAILURE);
    default:
      break;
  }
  if (wait(NULL) == -1) {
    return NULL;
  }
  if (unlink(tube_res) == -1) {
    return NULL;
  }
  if (unlink(tube_err) == -1) {
    return NULL;
  }
  return 0;
}

void mafct(int sig) {
  if (sig == SIGINT) {
    work = false;
  }
}

