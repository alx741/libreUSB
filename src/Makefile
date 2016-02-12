BINDIR = ../bin
CC = sdcc
CFLAGS = --use-non-free -V -mpic16 -p18f4550
SCRIPTS = ../scripts

firmware: example.hex
	cp example.hex $(BINDIR)/

uart.o: util/uart.c
	${CC} ${CFLAGS} -c util/uart.c

printf.o: util/printf.c uart.o
	${CC} ${CFLAGS} -c util/printf.c

usbcdc.o: usbcdc.c uart.o printf.o
	${CC} ${CFLAGS} -c usbcdc.c

example.o: example.c usbcdc.o
	${CC} ${CFLAGS} -c example.c

example.hex: example.o
	${CC} ${CFLAGS} example.o usbcdc.o uart.o printf.o


flash:
	$(SCRIPTS)/write_fw.sh

clean:
	rm -f $(BINDIR)/*
