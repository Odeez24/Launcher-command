#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#define TUBE_CL "TUBE_CLIENT_"
#define TUBE_RES "TUBE_RES_CLIENT_"
#define TUBE_ERR "TUBE_ERR_CLIENT_"

#ifndef PID_SIZE
#define PID_SIZE 32
#endif

#ifndef BUF_SIZE
#define BUF_SIZE 4096
#endif

#include <fcntl.h>

#include "file_sync.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(const int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "USAGE : client [cmd1]|[cmd2] |... |[cmdN]\n");
    return EXIT_SUCCESS;
  }
  char pid[PID_SIZE];
  int pidlen;
  if ((pidlen
        = snprintf(pid, PID_SIZE - 1, "%d",
          getpid())) < 0 || pidlen > PID_SIZE - 1) {
    return EXIT_FAILURE;
  }
  pid[pidlen] = '\0';
  char in_name[(int) strlen(TUBE_CL) + pidlen];
  strcpy(in_name, TUBE_CL);
  strcat(in_name, pid);
  char std_name[(int) strlen(TUBE_RES) + pidlen];
  strcpy(std_name, TUBE_RES);
  strcat(std_name, pid);
  char err_name[(int) strlen(TUBE_ERR) + pidlen];
  strcpy(err_name, TUBE_ERR);
  strcat(err_name, pid);
  if (mkfifo(in_name, S_IWUSR | S_IRUSR) == -1) {
    perror("mkfifo");
    return EXIT_FAILURE;
  }
  if (mkfifo(std_name, S_IWUSR | S_IRUSR) == -1) {
    perror("mkfifo");
    return EXIT_FAILURE;
  }
  if (mkfifo(err_name, S_IWUSR | S_IRUSR) == -1) {
    perror("mkfifo");
    return EXIT_FAILURE;
  }
  if (enfiler(getpid()) == -1) {
    fprintf(stderr, "%s : Impossible d'enfiler la valeur.\n", argv[0]);
    return EXIT_FAILURE;
  }
  int in_pipe;
  int std_pipe;
  int err_pipe;
  if ((in_pipe = open(in_name, O_WRONLY)) == -1) {
    perror("open");
    return EXIT_FAILURE;
  }
  if (unlink(in_name) == -1) {
    perror("unlink");
    return EXIT_FAILURE;
  }
  for (int k = 1; k < argc; k += 1) {
    if (write(in_pipe, argv[k], strlen(argv[k])) == -1) {
      perror("write");
      return EXIT_FAILURE;
    }
    if (k < argc - 1) {
      if (write(in_pipe, " ", 1) == -1) {
        perror("write");
        return EXIT_FAILURE;
      }
    }
  }
  if (close(in_pipe) == -1) {
    perror("close");
    return EXIT_FAILURE;
  }
  if ((std_pipe = open(std_name, O_RDONLY)) == -1) {
    perror("open");
    return EXIT_FAILURE;
  }
  if ((err_pipe = open(err_name, O_RDONLY)) == -1) {
    perror("open");
    return EXIT_FAILURE;
  }
  if (unlink(err_name) == -1) {
    perror("unlink");
    return EXIT_FAILURE;
  }
  if (unlink(std_name) == -1) {
    perror("unlink");
    return EXIT_FAILURE;
  }
  char buf[BUF_SIZE];
  ssize_t r;
  while ((r = read(std_pipe, buf, BUF_SIZE)) > 0) {
    if (write(STDOUT_FILENO, buf, (size_t) r) == -1) {
      exit(EXIT_FAILURE);
    }
  }
  while ((r = read(err_pipe, buf, BUF_SIZE)) > 0) {
    if (write(STDOUT_FILENO, buf, (size_t) r) == -1) {
      exit(EXIT_FAILURE);
    }
  }
  if (close(std_pipe) == -1) {
    perror("close");
    return EXIT_FAILURE;
  }
  if (close(err_pipe) == -1) {
    perror("close");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
