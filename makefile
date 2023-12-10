CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings -pthread \
  -O2 \

vpath %.c
vpath %.h
makefile_indicator = .\#makefile\#

exe1: ex1
obj1: main1.o

obj2: main2.o
exe2: ex2

obj3: main3.o
exe3: calc_int


.PHONY: all clean

all: $(executable)

$(exe1): $(obj1)
	$(CC) $(obj1) -o $(exe1)
main1.o: ex11.c


$(exe2): $(obj2)
	$(CC) $(obj2) -o $(exe2)
main2.o: ex2.c


clean:
	$(RM) $(obj1) $(obj2) $(obj3) $(obj4) $(exe1) $(exe2) $(exe3) $(exe4)
	@$(RM) $(makefile_indicator)

include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)


