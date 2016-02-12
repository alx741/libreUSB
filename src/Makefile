BINDIR = ../bin
CC = sdcc
CFLAGS = --use-non-free -V -mpic16 -p18f4550
SCRIPTS = ../scripts

firmware:
	${CC} ${CFLAGS} -o $(BINDIR)/ -c example.c
	${CC} ${CFLAGS} -o $(BINDIR)/ -c usbcdc.c
	${CC} ${CFLAGS} -o $(BINDIR)/ -c util/uart.c
	${CC} ${CFLAGS} -o $(BINDIR)/ -c util/printf.c
	cd $(BINDIR)/ && ${CC} ${CFLAGS} example.o usbcdc.o uart.o printf.o

flash:
	$(SCRIPTS)/write_fw.sh

clean:
	rm -f $(BINDIR)/*
