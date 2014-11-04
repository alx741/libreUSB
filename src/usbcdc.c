/*
 * File: 	usb.c
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file implements a USB/CDC device firmware
 *     for PIC18F4550 microcontroller, it allows to create
 *     a virtual serial (COM) port for data transmission with
 *     the PC over USB
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
#include "usb_cdc.h"
#include "usb_pic.h"
#include "usbcdc.h"



/************************************************
  DEVICE CURRENT STATE
 ***********************************************/

volatile unsigned char USB_DEVICE_STATE;
volatile unsigned char USB_DEVICE_ADDRESS;
volatile unsigned char USB_DEVICE_CURRENT_CONFIGURATION;

/************************************************
 ***********************************************
 ***********************************************/



/************************************************
  USB handling functions
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
        static void control_transfer_handler(void);

            // Requests handling
            static void handle_request_get_status(void);
            static void handle_request_clear_feature(void);
            static void handle_request_set_feature(void);
            static void handle_request_set_address(void);
            static void handle_request_get_descriptor(void);
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





/*********************************************************
    DESCRIPTORS definition 

    This descriptors describe a CDC/ACM device with 1
    configuration only

 *******************************************************/

/* DEVICE DESCRIPTOR

    This is a CDC Device class
    with one configuration only
*/
__code USB_DESCRIPTOR_DEVICE_t DEVICE_DESCRIPTOR;

DEVICE_DESCRIPTOR.bLength = sizeof(USB_DESCRIPTOR_DEVICE_t); // Total device descriptor size
DEVICE_DESCRIPTOR.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE; // Device descriptor
DEVICE_DESCRIPTOR.bcdUSB = USB_DEVICE_BCDUSB; // USB 2.0 compliant device
DEVICE_DESCRIPTOR.bDeviceClass = USB_CDC_CLASS_DEVICE; // CDC device Class
DEVICE_DESCRIPTOR.bDeviceSubClass = 0x00; // No device SubClass
DEVICE_DESCRIPTOR.bDeviceProtocol = 0x00; // No device Protocol
DEVICE_DESCRIPTOR.bMaxPacketSize0 = EP0_OUT_BUFFER_SIZE; // Max End Point 0 packet size is End Point 0 Buffer size
DEVICE_DESCRIPTOR.idVendor = 0x04D8; // Using the "Microchip" Vendor ID
DEVICE_DESCRIPTOR.idProduct = 0x0111 // Using arbitrary Product ID
DEVICE_DESCRIPTOR.bcdDevice = 0x0000 // No device version
DEVICE_DESCRIPTOR.iManufacturer = 0x01 // Manufacturer description text
DEVICE_DESCRIPTOR.iProduct = 0x02 // Product description text
DEVICE_DESCRIPTOR.iSerialNumber = 0x00 // No serial number text
DEVICE_DESCRIPTOR.bNumConfigurations = 0x01 // This device has one configuration only





/* CONFIGURATION DESCRIPTOR

    This device has one configuration only

    This a USB/CDC (Communications Device Class) configuration

    The configuration descriptor should contain the complete 
    interface, endpoint and class specific descriptors hierarchy

    CONFIGURATION (USB/CDC):
        - Configuration Descriptor
            - Interface Descriptor (Communications)
                - Functional Desciptor (Header)
                - Functional Desciptor (ACM)
                - Functional Desciptor (Union)
                - Functional Desciptor (Call Management)
                - EndPoint Desciptor (Notification Element)
            - Interface Desciptor (Data)
                - EndPoint Desciptor (Data Out)
                - EndPoint Desciptor (Data In)

*/

typedef struct
{
    USB_DESCRIPTOR_CONFIGURATION_t                                              CONFIGURATION_DESCRIPTOR;

        USB_DESCRIPTOR_INTERFACE_t                                                  INTERFACE_DESCRIPTOR_COMMUNICATIONS;
            USB_CDC_DESCRIPTOR_FUNCTIONAL_HEADER_t                                      FUNCTIONAL_DESCRIPTOR_HEADER;
            USB_CDC_DESCRIPTOR_FUNCTIONAL_ABSTRACT_CONTROL_MANAGEMENT_t                 FUNCTIONAL_DESCRIPTOR_ACM;
            USB_CDC_DESCRIPTOR_FUNCTIONAL_UNION_t                                       FUNCTIONAL_DESCRIPTOR_UNION;
            USB_CDC_DESCRIPTOR_FUNCTIONAL_CALL_MANAGEMENT_t                             FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT;
            USB_DESCRIPTOR_ENDPOINT_t                                                   ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT;

        USB_DESCRIPTOR_INTERFACE_t                                                  INTERFACE_DESCRIPTOR_DATA;
            USB_DESCRIPTOR_ENDPOINT_t                                                   ENDPOINT_DESCRIPTOR_OUT;
            USB_DESCRIPTOR_ENDPOINT_t                                                   ENDPOINT_DESCRIPTOR_IN; 
} CONFIGURATION_0_;




/*  CONFIGURATION  ( Contains whole configuration descriptors hierarchy )  */
__code CONFIGURATION_0_ CONFIGURATION_0;

// CONFIGURATION_DESCRIPTOR
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.bLength = sizeof(USB_DESCRIPTOR_CONFIGURATION_t); // Configuration Descriptor size
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION; // Configuration descriptor
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.wTotalLength = sizeof(CONFIGURATION_0_); // Whole configuration hierarchy size
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.bNumInterfaces = 0x02; // This configuration has 2 interfaces
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.bConfigurationValue = 0x01; // Index value for this configuration
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.iConfiguration = 0x00; // No configuration description text
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.bmAttributes = 0x80; // Bus Powered configuration
CONFIGURATION_0.CONFIGURATION_DESCRIPTOR.bMaxPower = 0x64; // This configuration takes up to 100mA from the bus

// INTERFACE_DESCRIPTOR_COMMUNICATIONS
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bLength = sizeof(USB_DESCRIPTOR_INTERFACE_t); // Interface Descriptor size
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE; // Interface descriptor
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bInterfaceNumber = 0x00; // This is interface 0
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bAlternateSetting = 0x00; // Alternate setting number
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bNumEndpoints = 0x01; // This interface has 1 endpoint
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bInterfaceClass = USB_CDC_CLASS_INTERFACE_COM; // Communications interface class
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bInterfaceSubClass = USB_CDC_SUBCLASS_INTERFACE_ACM; // Abstract Control Model sub class
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.bInterfaceProtocol = USB_CDC_PROTOCOL_INTERFACE_V250; // V250 protocol
CONFIGURATION_0.INTERFACE_DESCRIPTOR_COMMUNICATIONS.iInterface = 0x00; // No interface description text

// FUNCTIONAL_DESCRIPTOR_HEADER
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_HEADER.bFunctionalLength = sizeof(USB_CDC_DESCRIPTOR_FUNCTIONAL_HEADER_t); // Header FD size
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_HEADER.bDescriptorType = USB_CDC_FUNCTIONAL_CS_INTERFACE; // Interface descriptor type
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_HEADER.bDescriptorSubType = USB_CDC_FUNCTIONAL_HEADER; // Header FD
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_HEADER.bcdCDC = USB_CDC_HEADER_BCDUSB; // USB CDC 1.2 compliant

// FUNCTIONAL_DESCRIPTOR_ACM
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bFunctionalLength = sizeof(USB_CDC_DESCRIPTOR_FUNCTIONAL_ABSTRACT_CONTROL_MANAGEMENT_t); // ACM FD size
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bDescriptorType = USB_CDC_FUNCTIONAL_CS_INTERFACE; // Interface descriptor type
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bDescriptorSubType = USB_CDC_FUNCTIONAL_ACM; // ACM FD
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bmCapabilities = USB_CDC_ACM_BMCAPABILITIES; // Support "line" requests

// FUNCTIONAL_DESCRIPTOR_UNION
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bFunctionalLength = sizeof(USB_CDC_DESCRIPTOR_FUNCTIONAL_UNION_t); // Union FD size
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bDescriptorType = USB_CDC_FUNCTIONAL_CS_INTERFACE; // Interface descriptor type
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bDescriptorSubType = USB_CDC_FUNCTIONAL_UNION; // Union FD
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bControlInterface = 0x00; // Interface 0 is the control interface (communications class)
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_ACM.bSubordinateInterface0 = 0x01; // Interface 1 is subordinate (data class)

// FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT.bFunctionalLength = sizeof(USB_CDC_DESCRIPTOR_FUNCTIONAL_CALL_MANAGEMENT_t) // CM FD size
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT.bDescriptorType = USB_CDC_FUNCTIONAL_CS_INTERFACE; // Interface descriptor type
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT.bDescriptorSubType = USB_CDC_FUNCTIONAL_CALL_MANAGEMENT; // CM FD
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT.bmCapabilities = USB_CDC_CALL_MANAGEMENT_BMCAPABILITIES; // Don't handle call management
CONFIGURATION_0.FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT.bDataInterface = 0x01; // Interface 1 is data class

// ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT.bLength = sizeof(USB_DESCRIPTOR_ENDPOINT_t); // Endpoint descriptor size
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT; // Endpoint descriptor
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT.bEndpointAddress = USB_ENDPOINT_02_IN; // In endpoint 2
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT.bmAttributes = USB_ENDPOINT_INTERRUPT; // Interrupt endpoint
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT.wMaxPacketSize = 0x40; // 64 bytes max packet
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT.bInterval = 0x02; // Poll every 2 milliseconds

// INTERFACE_DESCRIPTOR_DATA
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bLength = sizeof(USB_DESCRIPTOR_INTERFACE_t); // Interface descriptor size
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE; // Interface descriptor
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bInterfaceNumber = 0x01; // This is interface 1
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bAlternateSetting = 0x00 // Alternate setting number
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bNumEndpoints = 0x02 // This interface has 2 endpoints
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bInterfaceClass = USB_CDC_CLASS_INTERFACE_DAT; // Data interface class
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bInterfaceSubClass = USB_CDC_SUBCLASS_INTERFACE_NONE; // No interface class
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.bInterfaceProtocol = USB_CDC_PROTOCOL_INTERFACE_NONE; // No interface protocol
CONFIGURATION_0.INTERFACE_DESCRIPTOR_DATA.iInterface = 0x00; // No interface description text

// ENDPOINT_DESCRIPTOR_OUT
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_OUT.bLength = sizeof(USB_DESCRIPTOR_ENDPOINT_t); // Endpoint descriptor size
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_OUT.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT; // Endpoint descriptor
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_OUT.bEndpointAddress = USB_ENDPOINT_03_OUT; // Out endpoint 3
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_OUT.bmAttributes = USB_ENDPOINT_BULK; // Bulk endpoint
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_OUT.wMaxPacketSize = 0x40; // 64 bytes max packet
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_OUT.bInterval = 0x00; // Poll as fast as possible

// ENDPOINT_DESCRIPTOR_IN
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_IN.bLength = sizeof(USB_DESCRIPTOR_ENDPOINT_t); // Endpoint descriptor size
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_IN.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT; // Endpoint descriptor
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_IN.bEndpointAddress = USB_ENDPOINT_03_IN; // In endpoint 3
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_IN.bmAttributes = USB_ENDPOINT_BULK; // Bulk endpoint
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_IN.wMaxPacketSize = 0x40; // 64 bytes max packet
CONFIGURATION_0.ENDPOINT_DESCRIPTOR_IN.bInterval = 0x00; // Poll as fast as possible




/* STRING DESCRIPTORS

   2 bytes coding caracters

   String descriptor 0 = Supported Languages
   String descriptor 1 = iManufacturer
   String descriptor 2 = iProduct
*/

__code unsigned char STRING_DESCRIPTOR_0[] =
{
    0x04,                           // bLength
    USB_DESCRIPTOR_TYPE_STRING,     // bDescriptorType
    0x09, 0x04                      //wLANGID[0] (English) ( usb languages specification (page 5) )
};


__code unsigned char STRING_DESCRIPTOR_1[] =
{
    // String: "Silly-Bytes"

    0x24,                           // bLength
    USB_DESCRIPTOR_TYPE_STRING,     // bDescriptorType
    'S', 0x00,                      // bString
    'i', 0x00,
    'l', 0x00,
    'l', 0x00,
    'y', 0x00,
    '-', 0x00,
    'B', 0x00,
    'y', 0x00,
    't', 0x00,
    'e', 0x00,
    's', 0x00
};


__code unsigned char STRING_DESCRIPTOR_2[] =
{
    // String: "Virtual COM port"

    0x34,                           // bLength
    USB_DESCRIPTOR_TYPE_STRING,     // bDescriptorType
    'V', 0x00,                      // bString
    'i', 0x00,
    'r', 0x00,
    't', 0x00,
    'u', 0x00,
    'a', 0x00,
    'l', 0x00,
    ' ', 0x00,
    'C', 0x00,
    'O', 0x00,
    'M', 0x00,
    ' ', 0x00,
    'p', 0x00,
    'o', 0x00,
    'r', 0x00,
    't', 0x00
};



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
}



/* Handles USB requests, states and transactions
 
  Always call this function using an IRQ like this:
 
  	void usb_isr(void) __shadowregs __interrupt 1
  	{
  		if( PIR2bits.USBIF )
  		{
  			usb_handler();
  			PIR2bits.USBIF = 0;
  		}
  	}
 
 */
void usb_handler(void)
{
    // If the device isn't in powered state avoid
    // interrupt hanling
    if( USB_DEVICE_STATE < USB_STATE_POWERED )
    {
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
            UCONbits.SUSPND = 0;
            UIRbits.IDLEIF = 0;
        }
        else
        {
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

/* Handles wake up events */
static void handle_actvif(void) 
{
    // Resume power to SIE
    UCONbits.SUSPND = 0;
    
    // Clear ACTVIF
    while( UIRbits.ACTVIF ){ UIRbits.ACTVIF = 0; }
}


/* Handles suspend events */
static void handle_idleif(void)
{
    // Suspend power to SIE
    UCONbits.SUSPND = 1;
}


/* Handles reset events */
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
}


/* Handles transactions by endpoint number */
static void handle_trnif(void)
{ 
    // Transactions to ENDPOINT 0 (Control Transfers)
    if( USTATbits.ENDP == 0 )
    {
        control_transfer_handler();
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


/* Handles control transfers
   
   Optional SET_DESCRIPTOR request not implemented here,
   so we don't need to worry about any DATA OUT STAGE.

   We handle this 3 control transfer stages:
        - OUT direction transactions
            * SETUP stage
            * STATUS stage
        - IN direction transactions
            * DATA IN stage
*/
static void control_transfer_handler(void)
{
    // Pending data to send during the control transfer
    static unsigned char *data_to_send; 

    // Pending number of bytes to send during the control transfer
    static unsigned int bytes_to_send;



    /*****  OUT direction transactions  (SETUP or STATUS stage)  *****/ 
	if (USTATbits.DIR == 0) {

        /*** SETUP transaction (SETUP stage) ***/
		if (ENDPOINT0_OUT.STAT.PID == USB_PID_TOKEN_SETUP) {

            // The cpu owns the endpoint 0 buffer descriptors
            ENDPOINT0_IN.STAT.UOWN = 0;
            ENDPOINT0_OUT.STAT.UOWN = 0;
            

            /* Handle requests */

            // GET_DESCRIPTOR
            if ( SETUP_PACKET.bRequest == USB_REQUEST_GET_DESCRIPTOR )
            { 
                handle_request_get_descriptor();
            }

        } 
    }
}



/***************  Requests Handling  *************/


/* Handle GET_DESCRIPTOR request

   DESCRIPTOR pointer will point to the first byte
   of the requested descriptor

   SIZE will contain the total size in bytes of
   the requested descriptor
*/
static void handle_request_get_descriptor(unsigned char *descriptor, unsigned int size)
{
    PORTBbits.RB3 = 1;
}
