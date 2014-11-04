CC = sdcc
PARAMETERS = --use-non-free -mpic16 -p18f4550

firmware:
	${CC} ${PARAMETERS} -c example.c
	${CC} ${PARAMETERS} -c usbcdc.c
	${CC} ${PARAMETERS} example.o usbcdc.o


clean:
	rm example.asm example.cod example.o example.lst example.hex usbcdc.o usbcdc.asm usbcdc.cod usbcdc.lst

