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

// Taille de notre tampon
#define BUF_SIZE 10
// Nom du segment de mémoire partagé
#define NOM_SHM "/le_shm_de_la_file_sync_31122023"

// Nos variables en mémoire partagée.
struct fifo {
  sem_t mutex;
  sem_t vide;
  sem_t plein;
  size_t tete;      // Position d'ajout dans le tampon
  size_t queue;     // Position de suppression dans le tampon
  pid_t buffer[];   // Le tampon contenant les données
};

// L'en-tête du segment de mémoire partagée
struct fifo *fifo_p = NULL;

#define TAILLE_SHM (sizeof(struct fifo) + BUF_SIZE)

int create_file_sync(void) {
  int shm_fd = shm_open(NOM_SHM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    return -1;
  }
  if (shm_unlink(NOM_SHM) == -1) {
    return -1;
  }
  if (ftruncate(shm_fd, TAILLE_SHM) == -1) {
    return -1;
  }
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED,
      shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    return -1;
  }
  fifo_p = (fifo *) shm_ptr;
  // Initialisation des variables
  if (sem_init(&fifo_p->mutex, 1, 1) == -1) {
    return -1;
  }
  if (sem_init(&fifo_p->vide, 1, BUF_SIZE) == -1) {
    return -1;
  }
  if (sem_init(&fifo_p->plein, 1, 0) == -1) {
    return -1;
  }
  fifo_p->tete = 0;
  fifo_p->queue = 0;
  return 0;
}


int destroy_file(void){
  int shm_fd = shm_open(NOM_SHM, O_RDWR,  S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    return -1;
  }
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ, MAP_SHARED,
      shm_fd, 0);
  fifo_p = (fifo *) shm_ptr;
  if (sem_destroy(&fifo_p->mutex) == -1) {
    return -1;
  }
  if (sem_destroy(&fifo_p->plein) == -1) {
    return -1;
  }
  if (sem_destroy(&fifo_p->vide) == -1) {
    return -1;
  }
  if (shm_unlink(NOM_SHM) == -1) {
    return -1;
  }
  return 0;
}

int enfiler(pid_t donnee) {
  int shm_fd = shm_open(NOM_SHM, O_RDWR,  S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    return -1;
  }
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ, MAP_SHARED,
      shm_fd, 0);
  fifo_p = (fifo *) shm_ptr;
  if (sem_wait(&fifo_p->vide) == -1) {
   return -1;
  }
  if (sem_wait(&fifo_p->mutex) == -1) {
    return -1;
  }
  fifo_p->buffer[fifo_p->tete] = donnee;
  fifo_p->tete = (fifo_p->tete + 1) % BUF_SIZE;
  if (sem_post(&fifo_p->mutex) == -1) {
    return -1;
  }
  if (sem_post(&fifo_p->plein) == -1) {
    return -1;
  }
  return 0;
}


pid_t defiler(void) {
  printf("Passage file 1\n");
  int shm_fd = shm_open(NOM_SHM, O_RDWR,  S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
     printf("Passage file err 1\n");
    return -1;
  }
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ, MAP_SHARED,
      shm_fd, 0);
  fifo_p = (fifo *) shm_ptr;
  if (sem_wait(&fifo_p->plein) == -1) {
    return -1;
  }
  if (sem_wait(&fifo_p->mutex) == -1) {
    return -1;
  }
  pid_t donnee = fifo_p->buffer[fifo_p->queue];
  fifo_p->queue = (fifo_p->queue + 1) % BUF_SIZE;
  if (sem_post(&fifo_p->mutex) == -1) {
    return -1;
  }
  if (sem_post(&fifo_p->vide) == -1) {
    return -1;
  }
  return donnee;
}
