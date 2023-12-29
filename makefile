file_dir = ./file_sync/
pars_dir = ./parseur/
CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings -pthread \
  -O2 \
  -I$(file_dir) -I$(pars_dir)

vpath %.c $(file_dir) $(pars_dir)
vpath %.h $(file_dir) $(pars_dir)
makefile_indicator = .\#makefile\#

objectslanceur = file_sync.o lanceur.o parseur.o
exelanceur = lanceur

objectsclient = file_sync.o client.o
execlient = client

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objectslanceur) $(exelanceur) $(objectsclient) $(execlient)
	@$(RM) $(makefile_indicator)

$(exelanceur): $(objectslanceur)
	$(CC) $(objectslanceur) -o $(exelanceur)

$(execlient): $(objectsclient)
	$(CC) $(objectsclient) -o $(execlient)

file_sync.o: file_sync.c file_sync.h
parseur.o: parseur.c parseur.h
lanceur.o: lanceur.c file_sync.h parseur.h
client.o: client.c file_sync.h


include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)


