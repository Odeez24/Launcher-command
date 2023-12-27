#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#define TUBE_CL "TUBE_CLIENT_"
#define TUBE_RES "TUBE_RES_CLIENT_"
#define TUBE_ERR "TUBE_ERR_CLIENT_"

#ifndef PID_SIZE
#define PID_SIZE 51
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
    char pipe_name[PID_SIZE];
    char pid[PID_SIZE];
    int pidlen;
    if ((pidlen = snprintf(pid, 50, "%d", getpid())) < 0
        || pidlen > 50) {
        return EXIT_FAILURE;
    }
    strcat(pipe_name, TUBE_CL);
    strcat(pipe_name, pid);

    if (mkfifo(pipe_name, S_IWUSR) == -1) {
        perror("mkfifo");
        return EXIT_FAILURE;
    }
    if (unlink(pipe_name) == -1) {
        perror("unlink");
        return EXIT_FAILURE;
    }

    const int fd_pipe = open(pipe_name, O_WRONLY);
    if (fd_pipe == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (enfiler(getpid()) == -1) {
        fprintf("%s : Impossible d'enfiler la valeur.\n", argv[0]);
        return EXIT_FAILURE;
    }

    char std_name[PIPE_SIZE];
    strcat(std_name, TUBE_RES);
    strcat(std_name, pid);

    char err_name[PIPE_SIZE];
    strcat(err_name, TUBE_RES);
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
