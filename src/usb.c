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
#include "usb.h"
#include "usb_pic.h"



/************************************************
 * DEVICE CURRENT STATE
 ***********************************************/

volatile unsigned char USB_DEVICE_STATE;
volatile unsigned char USB_DEVICE_ADDRESS;
volatile unsigned char USB_DEVICE_CURRENT_CONFIGURATION;

/************************************************
 ***********************************************
 ***********************************************/



/************************************************
 * USB handling
 ***********************************************/

// General usb Handler
void usb_handler(void);

    // Interrupt handling (pic18f4550 datasheet, page 178)
    static void handle_actvif(void); 
    static void handle_idleif(void); 
    static void handle_stallif(void); 
    static void handle_uerrif(void); 
    static void handle_sofif(void); 
    static void handle_urstif(void); 

    // Transactions handling (By endpoint)
    static void handle_trnif(void); 

        // Control transfers handling
        static void handle_control_transfer(void);

            // Requests handling
            static void handle_request_get_status(void);
            static void handle_request_clear_feature(void);
            static void handle_request_set_feature(void);
            static void handle_request_set_address(void);
            static void handle_request_get_descriptor(void);
            static void handle_request_set_descriptor(void);
            static void handle_request_get_configuration(void);
            static void handle_request_set_configuration(void);
            static void handle_request_get_interface(void);
            static void handle_request_set_interface(void);


/************************************************
 ***********************************************
 ***********************************************/





/*********************************************************
    ENDPOINT 0 definition 

    Enpoint 0 out buffer starts at 0500h in data memory,
    is 8 bytes long [0500h - 0507h] 

    Enpoint 1 out buffer starts at 0508h in data memory,
    is 64 bytes long [0508h - 056Bh] 
 *******************************************************/

// EP0 Buffer size in bytes
#define EP0_OUT_BUFFER_SIZE 8 // SetUp packet size
#define EP0_IN_BUFFER_SIZE 64

// Endpoint 0 buffers location
#define ENDPOINT0_OUT_BUFFER  0x0500
#define ENDPOINT0_IN_BUFFER  0x0508

// Endpoint 0 buffer descritors allocation
volatile BUFFER_DESCRIPTOR_t __at(0x0400 + (0 * 8)) ENDPOINT0_OUT;
volatile BUFFER_DESCRIPTOR_t __at(0x0404 + (0 * 8)) ENDPOINT0_IN;

// Setup Packet is allocated in the out endpoint 0 buffer
volatile USB_SETUP_PACKET_t __at(ENDPOINT0_OUT_BUFFER) SETUP_PACKET;


/******************************************************
 ******************************************************
 ******************************************************/




/* Initializes the USB hardware */
void usb_init(void)
{
	// Set current device state
	USB_DEVICE_STATE = USB_STATE_DETACHED;
    USB_DEVICE_ADDRESS = 0x00;
    USB_DEVICE_CURRENT_CONFIGURATION = 0x00;

	// Clear all usb related registers
	UCON = 0;
	UCFG = 0;
	UIR = 0;
	UIE = 0;
	UEIR = 0;
	UEIE = 0;

    // Disable all Endpoints
	UEP0 = 0; UEP1 = 0;	UEP2 = 0; UEP3 = 0;	UEP4 = 0; UEP5 = 0;
	UEP6 = 0; UEP7 = 0;	UEP8 = 0; UEP9 = 0;	UEP10 = 0; UEP11 = 0;
	UEP12 = 0; UEP13 = 0; UEP14 = 0; UEP15 = 0;

    // Device USB address 0
    UADDR = 0x00;

	// Enable internal pullup resistors and full speed mode
	UCFGbits.UPUEN = 1;
	UCFGbits.FSEN = 1;

	// Enable usb module
	UCONbits.USBEN = 1;
	USB_DEVICE_STATE = USB_STATE_ATTACHED;

    // Wait for initial SE0 condition to clear
    while( UCONbits.SE0 );

    // Device is now powered
    USB_DEVICE_STATE = USB_STATE_POWERED;

    // Enable interrupts
    INTCON=0xC0;
    RCONbits.IPEN = 0; // No interrupts priority levels
    UIEbits.ACTVIE = 1; // Activity interrupt
    UIEbits.IDLEIE = 1; // Idle interrupt
    UIEbits.STALLIE = 1; // Stall interrupt
    UIEbits.UERRIE = 1; // USB errors interrupt
    UIEbits.SOFIE = 1; // Start of frame interrupt
    UIEbits.TRNIE = 1; // Transaction finish interrupt
    UIEbits.URSTIE = 1; // Reset interrupt
    PIE2bits.USBIE = 1; // USB interrupts

    PORTBbits.RB7 = 1; 
}



/* Handles USB requests, states and transactions
 *
 * Always call this function using an IRQ like this:
 *
 * 	void usb_isr(void) __shadowregs __interrupt 1
 * 	{
 * 		if( PIR2bits.USBIF )
 * 		{
 * 			usb_handler();
 * 			PIR2bits.USBIF = 0;
 * 		}
 * 	}
 *
 */
void usb_handler(void)
{
    // If the device isn't in powered state avoid
    // interrupt hanling
    if( USB_DEVICE_STATE < USB_STATE_POWERED )
    {
        PORTBbits.RB6 = 1; 
        return;
    }

	// Activity in the bus has been detected
	if( UIRbits.ACTVIF && UIEbits.ACTVIE )
    {
        handle_actvif();
        UIRbits.ACTVIF = 0;
	}

	// A idle condition has been detected
	if( UIRbits.IDLEIF && UIEbits.IDLEIE )
    {
        // Do not suspend if not adressedd
        if( USB_DEVICE_STATE < USB_STATE_ADDRESS )
        {
            PORTBbits.RB0 = 1;
            UCONbits.SUSPND = 0;
            UIRbits.IDLEIF = 0;
        }
        else
        {
            PORTBbits.RB1 = 1;
            handle_idleif();
            UIRbits.IDLEIF = 0;
        }
	}

	// A stall condition has been detected
	if( UIRbits.STALLIF && UIEbits.STALLIE )
    {
        // Do nothing
        UIRbits.STALLIF = 0;
	}

	// A error condition has been detected
	if( UIRbits.UERRIF && UIEbits.UERRIE )
    {
        // Do nothing
        // TODO: I guess we should do 
        //       something about this...

        // UERRIF is Read Only, clear UEIR instead
        UEIR = 0;
	}

	// A start of frame has been detected
	if( UIRbits.SOFIF && UIEbits.SOFIE )
    {
        // Do nothing
        UIRbits.SOFIF = 0;
	}

	// A transaction has finished
	if( UIRbits.TRNIF && UIEbits.TRNIE )
    {
        handle_trnif();
        UIRbits.TRNIF = 0;
	}

	// A reset signal has been achived
	if( UIRbits.URSTIF && UIEbits.URSTIE )
    {
        handle_urstif();
        UIRbits.URSTIF = 0;
	} 
}




/***************  interrupt Handling  *************/

static void handle_actvif(void) 
{
    // Resume power to SIE
    UCONbits.SUSPND = 0;
    
    // Clear ACTVIF
    while( UIRbits.ACTVIF ){ UIRbits.ACTVIF = 0; }
}


static void handle_idleif(void)
{
    // Suspend power to SIE
    UCONbits.SUSPND = 1;
}


static void handle_urstif(void) 
{ 
    // Wipe interrupt flags
    UIR = 0x00;
    UEIR = 0x00;

    // Enpoint 0 configuration
    UEP0bits.EPINEN = 1; // Endpoint 0 IN enabled
    UEP0bits.EPOUTEN = 1; // Endpoint 0 OUT enabled
    UEP0bits.EPHSHK = 1; // Endpoint 0 handshake enabled
    UEP0bits.EPCONDIS = 0; // Endpoint 0 Control transfers allowed
    UEP0bits.EPSTALL = 0; // Endpoint 0 is not stalled

    // Flush transactions queue
    while( UIRbits.TRNIF ){ UIRbits.TRNIF = 0; }

    // Enable SIE packet processing
    UCONbits.PKTDIS = 0;

    // Clear endpoin 0 buffer descriptors STAT
    ENDPOINT0_OUT.STAT.stat = 0x00;
    ENDPOINT0_IN.STAT.stat = 0x00;

    // Configure endpoint 0 buffer descriptors
    ENDPOINT0_OUT.ADDR = ENDPOINT0_OUT_BUFFER; // Buffer memory address for out ep0
    ENDPOINT0_IN.ADDR = ENDPOINT0_IN_BUFFER; // Buffer memory address for in ep0 
    ENDPOINT0_OUT.CNT = EP0_OUT_BUFFER_SIZE; // Recive up to EP0_OUT_BUFFER_SIZE bytes
    ENDPOINT0_OUT.STAT.UOWN = 1; // Give out buffer descriptor control to the SIE
    ENDPOINT0_IN.STAT.UOWN = 0; // Give in buffer descriptor control to the CORE

    // Device is now in default state
    USB_DEVICE_STATE = USB_STATE_DEFAULT;

    PORTBbits.RB6 = 1;
}


/* Handles transactions by endpoint number */
static void handle_trnif(void)
{ 
    PORTBbits.RB4 = 1;
    // Transactions to ENDPOINT 0 (Control Transfers)
    if( USTATbits.ENDP == 0 )
    {
        handle_control_transfer();
    }

    /* PUT CLASS SPECIFIC EP HANDLERS HERE */

    // Transactions to ENDPOINT 1
    else if( USTATbits.ENDP == 1)
    {
    } 

    // Transactions to ENDPOINT 2
    else if( USTATbits.ENDP == 2)
    {
    } 



    // Transactions to ENDPOINT N 


}




/***************  Transfers Handling  *************/


/* Handles control transfers */
static void handle_control_transfer(void)
{
    // OUT transaction
	if (USTATbits.DIR == 0) {

        // SETUP transaction (SETUP stage)
		if (ENDPOINT0_OUT.STAT.PID == USB_PID_TOKEN_SETUP) {

            // The cpu owns the endpoint 0 buffer descriptors
            ENDPOINT0_OUT.STAT.UOWN = 0;
            ENDPOINT0_IN.STAT.UOWN = 0;
            PORTBbits.RB3 = 1;
            


            //if ( SETUP_PACKET.bRequest == USB_REQUEST_GET_DESCRIPTOR )
            //{ 
            //    return;
            //}
        } 
    }
}
