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

#define NOM_SHM "/mon_shm_a_moi_589422985365427"

// Taille de notre tampon
#define N 10

// Nos variables en mémoire partagée.
struct fifo {
  sem_t mutex;
  sem_t vide;
  sem_t plein;
  size_t tete;      // Position d'ajout dans le tampon
  size_t queue;     // Position de suppression dans le tampon
  char buffer[]; // Le tampon contenant les données
};

// L'en-tête du segment de mémoire partagée
struct fifo *fifo_p = NULL;

#define TAILLE_SHM (sizeof(struct fifo) + N)

void enfiler(char c);
char defiler(void);

void reader(void);
void writer(void);

int main(void) {
  int shm_fd = shm_open(NOM_SHM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_SUCCESS);
  }

  if (shm_unlink(NOM_SHM) == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  if (ftruncate(shm_fd, TAILLE_SHM) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  fifo_p = (struct fifo *) shm_ptr;

  // Initialisation des variables
  if (sem_init(&fifo_p->mutex, 1, 1) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&fifo_p->vide, 1, N) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&fifo_p->plein, 1, 0) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  fifo_p->tete = 0;
  fifo_p->queue = 0;

  // On se duplique
  switch (fork()) {
  case -1:
    perror("fork");
    exit(EXIT_FAILURE);
  case 0:
    reader();
    exit(EXIT_SUCCESS);
  default:
    writer();
  }

  if (wait(NULL) == -1) {
    perror("wait");
    exit(EXIT_FAILURE);
  }

  // Il faudrait prendre des précautions particulières afin d'être sûr
  // que les sémaphores sont détruits à la fin du processus (gestion des signaux).
  if (sem_destroy(&fifo_p->mutex) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (sem_destroy(&fifo_p->plein) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (sem_destroy(&fifo_p->vide) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (shm_unlink (NOM_SHM) == -1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

void reader(void) {
  ssize_t n;
  char c;
  while ((n = read(STDIN_FILENO, &c, 1)) > 0) {
    enfiler(c);
  }
  if (n == -1)  {
    perror("read");
    exit(EXIT_FAILURE);
  }

  enfiler(0);
}


void writer(void) {
  char c;
  do {
    c = defiler();
    errno = 0;
    if (write(STDOUT_FILENO, &c, 1) != 1) {
      if (errno != 0) {
        perror("write");
        exit(EXIT_FAILURE);
      } else {
        fprintf(stderr, "write: impossible d'écrire les données demandées");
        exit(EXIT_FAILURE);
      }
    }

  } while (c != 0);
}

void enfiler(char c) {
    if (sem_wait(&fifo_p->vide) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
    if (sem_wait(&fifo_p->mutex) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
    fifo_p->buffer[fifo_p->tete] = c;
    fifo_p->tete = (fifo_p->tete + 1) % N;

    if (sem_post(&fifo_p->mutex) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    if (sem_post(&fifo_p->plein) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
}

char defiler(void) {
    if (sem_wait(&fifo_p->plein) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
    if (sem_wait(&fifo_p->mutex) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }

    char c = fifo_p->buffer[fifo_p->queue];
    fifo_p->queue = (fifo_p->queue + 1) % N;

    if (sem_post(&fifo_p->mutex) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    if (sem_post(&fifo_p->vide) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }

    return c;
}
