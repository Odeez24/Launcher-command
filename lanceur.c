#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <file_sync.h>
#include <parseur.h>

//--- Marco tubes ------------------------------------------------------

#define TUBE_CL "TUBE_CLIENT_"
#define TUBE_RES "TUBE_RES_CLIENT_"
#define TUBE_ERR "TUBE_ERR_CLIENT_"
#define BUF_SIZE 4096
#define CMD_SIZE 64
#define PID_SIZE 32

//--- Outils pour les threads --------------------------------------------------
/*
 * Les arguments transmis au thread.
 */
struct my_thread_args {
  pid_t client; // Le pid_t du clint pour lequel le thread est crée
};

// run: fonction de lancement du thread
void *run(struct my_thread_args *a);
// Type de la fonction de lancement du thread
typedef void *(*start_routine_type)(void *);

//  mafct: fonction de gestion des signaux
void mafct(int sig);

//--- Main ---------------------------------------------------------------------
int main(void) {
  switch (fork()) {
    case -1:
      perror("fork");
      return EXIT_FAILURE;
    case 0:
      if (setsid() == -1){
        perror("setsid");
        exit(EXIT_FAILURE);
      }
      if (create_file_sync() == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
      }
      struct sigaction act;
      act.sa_handler = mafct;
      act.sa_flags = 0;
      sigemptyset(&act.sa_mask);
      if (sigaction(SIGTERM, &act, NULL) != 0) {
        exit(EXIT_FAILURE);
      }
      pid_t p;
      while ((p = defiler()) != -1) {
        pthread_t th;
        struct my_thread_args *a = malloc(sizeof(struct my_thread_args));
        if (a == NULL) {
          exit(EXIT_FAILURE);
        }
        a->client = p;
        if (pthread_create(&th, NULL, (start_routine_type) run, a) != 0) {
          exit(EXIT_FAILURE);
        }
      }
      break;
    default:
      break;
  }
  return EXIT_SUCCESS;
}

void *run(struct my_thread_args *a) {
  switch (fork()) {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
    case 0:
      {
        /* TEMP */
        //int log = open("log", O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
        //if (log == -1) {
        //exit(EXIT_FAILURE);
        //}
        /* FIN TEMP */
        int fd;
        int fd_res;
        int fd_err;
        char pid[PID_SIZE];
        int pidlen;
        if ((pidlen = snprintf(pid, PID_SIZE - 1, "%d", a->client)) < 0
            || pidlen > PID_SIZE - 1) {
          return NULL;
        }
        free(a);
        pid[pidlen] = '\0';
        char tube_cl[(int) strlen(TUBE_CL) + pidlen];
        strcpy(tube_cl, TUBE_CL);
        strcat(tube_cl, pid);
        if ((fd = open(tube_cl, O_RDONLY)) == -1) {
          perror("open cl");
          exit(EXIT_FAILURE);
        }
        char tube_res[(int) strlen(TUBE_RES) + pidlen];
        strcpy(tube_res, TUBE_RES);
        strcat(tube_res, pid);
        char tube_err[(int) strlen(TUBE_ERR) + pidlen];
        strcpy(tube_err, TUBE_ERR);
        strcat(tube_err, pid);
        char c;
        char buffer[BUF_SIZE];
        int nbarg = 0;
        int i = 0;
        while (read(fd, &c, sizeof(char)) > 0) {
          buffer[i] = c;
          if (c == ' ') {
            ++nbarg;
          }
          ++i;
        }
        buffer[i] = '\0';
        if (close(fd) == -1) {
          perror("close");
          exit(EXIT_FAILURE);
        }
        if ((fd_res = open(tube_res, O_WRONLY)) == -1) {
          perror("open res");
          exit(EXIT_FAILURE);
        }
        if ((fd_err = open(tube_err, O_WRONLY)) == -1) {
          perror("open err");
          exit(EXIT_FAILURE);
        }
        if (dup2(fd_res, STDOUT_FILENO) == -1) {
          perror("dup2");
          exit(EXIT_FAILURE);
        }
        if (close(fd_res) == -1) {
          perror("close");
          exit(EXIT_FAILURE);
        }
        if (dup2(fd_err, STDERR_FILENO) == -1) {
          perror("dup2");
          exit(EXIT_FAILURE);
        }
        if (close(fd_err) == -1) {
          perror("close");
          exit(EXIT_FAILURE);
        }
        char **opt = parseur_arg(buffer);
        execvp(opt[0], (char * const *) opt);
        //if (write(log, opt[0], strlen(opt[0])) == -1) {
        //exit(EXIT_FAILURE);
        //}
        //if (write(log, opt[1], strlen(opt[1])) == -1) {
        //exit(EXIT_FAILURE);
        //}
        dispose_arg(opt);
        fprintf(stderr, "Error during the execution of the command.\n");
        exit(EXIT_FAILURE);
      }
    default:
      if (wait(NULL) == -1) {
        perror("wait");
        exit(EXIT_FAILURE);
      }
  }
  return 0;
}

void mafct(int sig) {
  if (sig == SIGTERM) {
    if (destroy_file() == -1) {
      fprintf(stderr, "Error during destroy_file\n");
      exit(EXIT_FAILURE);
    }
  }
}
