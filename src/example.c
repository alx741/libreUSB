#include <pic18f4550.h>
#include "usb.h"

/*************************************************************************************************
  PIC18F4550 CONFIGURATION - Crystal used: 20MHz  (Datasheet page 286 - 295)


  USB freq: 20MHz, Microcontroller freq: 48MHz  (Datasheet page 30, table 2-3)
*************************************************************************************************/

// Divide by 5 (20MHz oscillator input), USB clock source come directly from the primary oscillator
#pragma config PLLDIV=5, USBDIV=2, CPUDIV=OSC1_PLL2

// High speed oscillator, PLL enabled (HSPLL)
#pragma config IESO=OFF, FCMEN=OFF, FOSC=HSPLL_HS

// Power-up timer enabled, Brown-out disabled, USB voltage regulator enabled
#pragma config PWRT=ON, BOR=OFF, VREGEN=ON

// Watch dog timer disabled
#pragma config WDT=OFF

// MCLR (pin 1) enabled, PORTB is digital, CCP2 i/o multiplexed with RC1
#pragma config MCLRE=ON, PBADEN=OFF, CCP2MX=ON

// Low voltage programming disabled, Debuging disabled
#pragma config DEBUG=OFF, STVREN=OFF, LVP=OFF, ICPRT=OFF, XINST=OFF

// Code read protection off
#pragma config CP0=OFF, CP1=OFF, CP2=OFF, CP3=OFF

// Data protection off
#pragma config CPB=OFF, CPD=OFF

// Code write protection off
#pragma config WRT0=OFF, WRT1=OFF, WRT2=OFF, WRT3=OFF

// Configuration protection off
#pragma config WRTC=OFF, WRTB=OFF, WRTD=OFF

// Data table read protection off
#pragma config EBTR0=OFF, EBTR1=OFF, EBTR2=OFF, EBTR3=OFF 

// Boot table read protection off
#pragma config EBTRB=OFF


/*************************************************************************************************
*************************************************************************************************/

void delay(unsigned int delay)
{
	unsigned int a = 0;
	unsigned int i=0;

	for(i=0;i<delay;i++)
	{
		while(a < 10)
		{
			a = a+1;
		}
		a=0;
	}
}


void usb_isr(void) __shadowregs __interrupt 1
{
    if( PIR2bits.USBIF )
    {
        usb_handler();
        PIR2bits.USBIF = 0;
    }
}


void main(void)
{
    // Oscillator config
    OSCCONbits.SCS = 0; // Primary (Crystal) oscillator (datasheet page 32)

    // All pins are digital (datasheet page 260)
    ADCON1bits.PCFG = 0xF;

    /*    DEBUG    */
    TRISB=0;
    PORTB=0;

    // Init USB
    usb_init();

    while(1)
    {
        // DO something
    } 
} 
