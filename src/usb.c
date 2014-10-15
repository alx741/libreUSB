/*
 * File: 	usb.c
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file implements usb.h
 * 	   defines usb funtions
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


#include <pic18f4550.h>



// DEVICE CURRENT STATE
unsigned char USB_DEVICE_STATE = NULL;
unsigned char USB_DEVICE_ADDRESS = NULL;
unsigned char USB_DEVICE_CURRENT_CONFIGURATION = NULL;



/* Initializes the USB hardware */
void usb_init(void)
{
	// Set current device state
	USB_DEVICE_STATE = USB_STATE_DETACHED;

	// Clear all usb related registers
	UCON = 0;
	UCFG = 0;
	UIR = 0;
	UIE = 0;
	UEP0 = 0; UEP1 = 0;	UEP2 = 0; UEP3 = 0;	UEP4 = 0; UEP5 = 0;
	UEP6 = 0; UEP7 = 0;	UEP8 = 0; UEP9 = 0;	UEP10 = 0; UEP11 = 0;
	UEP12 = 0; UEP13 = 0; UEP14 = 0; UEP15 = 0;

	// Enable internal pullup resistors and full speed mode
	UCFGbits.UPUEN = 1;
	UCFGbits.FSEN = 1;

	// Enable usb module
	UCONbits.USBEN = 1;
	USB_DEVICE_STATE = USB_STATE_ATTACHED;

	// If theres not a reset status
	if( (USB_DEVICE_STATE==USB_STATE_ATTACHED) && !UCONbits.SE0 )
	{
		// Enable some interrupts
		UIEbits.URSTIE = 1; // Reset interrupt
		UIEbits.IDLEIE = 1; // Idle interrupt
		UIEbits.ACTVIE = 1; // Activity interrupt
		PIE2bits.USBIE = 1; // USB interrupts

		// Device is now powered
		USB_DEVICE_STATE = USB_STATE_POWERED;
	}
}
