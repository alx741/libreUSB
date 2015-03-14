/*
 * File: 	usbcdc.h
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file contains USB/CDC interface functions declarations for
 * PIC18F4550 microcontroller device firmware. Allows to create a virtual
 * serial (COM) port for data transmission with the PC over USB
 *
 *
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _USBCDC_H
#define _USBCDC_H



/*
 * Initializes the USB hardware
 *
 * Wait until device is configured before start the actual communication with
 * the PC ( see usb_is_configured() )
 */
void usb_init(void);



/*
 * Handles USB requests, states and transactions
 *
 * Always call this function using an IRQ like this:
 *
 *	void usb_isr(void) __shadowregs __interrupt 1
 *	{
 *		if(PIR2bits.USBIF)
 *		{
 *			usb_handler();
 *			PIR2bits.USBIF = 0;
 *		}
 *	}
 */
void usb_handler(void);



/*
 * Returns a non-zero value if device has been enumerated by the pc, it's
 * configured and ready for send and receive data
 */
unsigned char usb_is_configured(void);



/*
 * Sends a character C to CDC virtual com port
 *
 * Returns a non-zero value if success
 */
unsigned char usb_cdc_putc(char c);



/*
 * Returns a character from CDC virtual com port
 *
 * Block until a character has been received
 */
char usb_cdc_getc(void);



/*
 * Sends a character string STR to CDC virtual com port
 *
 * Returns a non-zero value if success
 */
unsigned char usb_cdc_puts(char *str);



/*
 * Reads a character string STR from CDC virtual com port
 *
 * Returns a non-zero value if success
 */
unsigned char usb_cdc_gets(char *str);


#endif // _USBCDC_H
