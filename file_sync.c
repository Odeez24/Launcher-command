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
#iclude

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
  (pid_t) buffer[]; // Le tampon contenant les données
};

// L'en-tête du segment de mémoire partagée
struct fifo *fifo_p = NULL;

#define TAILLE_SHM (sizeof(struct fifo) + BUF_SIZE)

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

  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED,
    shm_fd, 0);
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

  if (sem_init(&fifo_p->vide, 1, BUF_SIZE) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&fifo_p->plein, 1, 0) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  fifo_p->tete = 0;
  fifo_p->queue = 0;


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

pid_t enfiler (pid_t donnee){

}

pid_t defiler (){

}
