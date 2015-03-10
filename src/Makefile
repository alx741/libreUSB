BINDIR = ../bin
CC = sdcc
PARAMETERS = --use-non-free -mpic16 -p18f4550

firmware:
	${CC} ${PARAMETERS} -o $(BINDIR)/ -c example.c
	${CC} ${PARAMETERS} -o $(BINDIR)/ -c usbcdc.c
	cd $(BINDIR)/ && ${CC} ${PARAMETERS} example.o usbcdc.o


clean:
	rm -f $(BINDIR)/*
