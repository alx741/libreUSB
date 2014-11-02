CC = sdcc
PARAMETERS = --use-non-free -mpic16 -p18f4550

firmware:
	${CC} ${PARAMETERS} -c example.c
	${CC} ${PARAMETERS} -c usb.c
	${CC} ${PARAMETERS} example.o usb.o


clean:
	rm example.asm example.cod example.o example.lst example.hex usb.o usb.asm usb.cod usb.lst

