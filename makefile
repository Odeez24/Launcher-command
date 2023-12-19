file_dir = ./file_sync/
CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings -pthread \
  -O2 \
  -I$(file_dir)

vpath %.c $(file_dir)
vpath %.h $(file_dir)
makefile_indicator = .\#makefile\#

objects: file.o lanceur.o 
executable = lanceur_commande

.PHONY: all clean

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

lanceur.o: lanceur.c file_sync.h
file.o: file_sync.c file_sync.h


clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)


