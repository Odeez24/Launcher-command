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

#ifndef PIPE_SIZE
#define PIPE_SIZE 254
#endif


#include <fcntl.h>

#include "file_sync.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(const int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, "USAGE : client [cmd1]|[cmd2] |... |[cmdN]");
        return EXIT_SUCCESS;
    }
    char pid[PID_SIZE];
    int pidlen;
    if ((pidlen = snprintf(pid, PID_SIZE - 1, "%d", getpid())) < 0
        || pidlen > PID_SIZE - 1) {
        return EXIT_FAILURE;
    }
    pid[pidlen] = '\0';
    char pipe_name[(int) strlen(TUBE_CL) + pidlen];
    strcpy(pipe_name, TUBE_CL);
    strcat(pipe_name, pid);
    if (mkfifo(pipe_name, S_IWUSR | S_IRUSR) == -1) {
        perror("mkfifo");
        return EXIT_FAILURE;
    }
    if (enfiler(getpid()) == -1) {
        fprintf(stderr, "%s : Impossible d'enfiler la valeur.\n", argv[0]);
        return EXIT_FAILURE;
    }
    const int fd_pipe = open(pipe_name, O_WRONLY);
    if (fd_pipe == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (unlink(pipe_name) == -1) {
        perror("unlink");
        return EXIT_FAILURE;
    }
    char std_name[(int) strlen(TUBE_RES) + pidlen];
    strcpy(std_name, TUBE_RES);
    strcat(std_name, pid);

    char err_name[(int) strlen(TUBE_ERR) + pidlen];
    strcpy(err_name, TUBE_RES);
    strcat(err_name, pid);
    if (open(std_name, O_RDONLY) == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (open(err_name, O_RDONLY) == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    for (int k = 1; k < argc; k += 1) {
        if (write(fd_pipe, argv[k], strlen(argv[k])) == -1) {
            perror("write");
            return EXIT_FAILURE;
        }
        if (k < argc - 1) {
            if (write(fd_pipe, " ", 1) == -1) {
                perror("write");
                return EXIT_FAILURE;
            }
        }
    }

    // TODO

    if( close(fd_pipe) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
