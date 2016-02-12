#include <pic18fregs.h>
#include "uart.h"


void uart_init(void)
{
    SPBRGH = 0;
	SPBRG = 77;
	TXSTAbits.BRGH = 0;
	BAUDCONbits.BRG16 = 0;
	TXSTAbits.SYNC = 0;
	RCSTAbits.SPEN = 1;
	PIE1bits.TXIE = 0;
	TXSTAbits.TX9 = 0;
	TXSTAbits.TXEN = 1;
	PIE1bits.RCIE = 0;
	RCSTAbits.RX9 = 0;
	TRISCbits.TRISC6 = 1;
	TRISCbits.TRISC7 = 1;
	RCSTAbits.CREN = 1;
}


void putchar(char c)
{
	while( ! PIR1bits.TXIF );
	TXREG = c;
}


int getchar(char* c)
{
    // If error reset CREN
	if (RCSTAbits.OERR)
	{
		RCSTAbits.CREN=0;
		RCSTAbits.CREN=1;
	}

	while( ! PIR1bits.RCIF ){ }

	*c = RCREG;
	return 1;
}


void puts(const char *s)
{
    char c = *s;
    while( c != '\0' )
    {
        putchar(c);
        c = *(++s);
    }
}
