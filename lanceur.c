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


//--- Outils pour les threads --------------------------------------------------
/*
 * Les arguments transmis au thread.
 */
struct my_thread_args {
  pid_t client; // Le pid_t du clint pour lequel le thread est crée
};

// run: fonction de lancement du thread
void * run(struct my_thread_args* a);

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
  if (create_file_sync() == -1){
    fprintf(stderr, "Error during create_file_sync");
    exit(EXIT_FAILURE);
  }
  int nbth = 0;
	struct sigaction act;
  act.sa_handler = mafct;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, NULL);
  pthread_t *th;
  while (work){
    pid_t p = defiler();
    struct my_thread_args* a = malloc(sizeof(struct my_thread_args));
    if (a == NULL) {
      perror("malloc()");
      exit(EXIT_FAILURE);
    }
    a->client = p;
    if (th == NULL){
      fprintf(stderr, "pthread_join: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    if ((errnum = pthread_create(th, NULL, (start_routine_type)run, a)) != 0) {
        fprintf(stderr, "Error: malloc");
        exit(EXIT_FAILURE);
    }
    ++nbth;
  }
  for (int i = 0; i < nbth; ++i){
    int *ptr;
    if ((errnum = pthread_join(*th, (void **) &ptr)) != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    free(ptr);
  }
  if (destroy_file() == -1){
    fprintf(stderr, "Error during destroy_file");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

void *run(struct my_thread_args* a) {
  int fd;
  char pid[UCHAR_MAX];
  sprintf(pid, "%d", a->client);
}

void mafct (int sig) {
	if (sig == SIGINT) {work = false;}
}
