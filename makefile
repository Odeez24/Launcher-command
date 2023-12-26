file_dir = ./file_sync/
CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings -pthread \
  -O2 \
  -I$(file_dir)

vpath %.c $(file_dir)
vpath %.h $(file_dir)
objects = file_sync.o lanceur.o client.o
executable = lanceur_commande
makefile_indicator = .\#makefile\#

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)


file_sync.o: file_sync.c file_sync.h
lanceur.o: lanceur.c file_sync.h
client.o: file_sync.h



include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)


