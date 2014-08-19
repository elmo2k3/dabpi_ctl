LDFLAGS=-lwiringPi
#CFLAGS=-Wall -Werror -Wextra
dabpi_ctl: dabpi_ctl.o si46xx.o

.PHONY: clean

clean:
	rm -f dabpi_ctl *.o
