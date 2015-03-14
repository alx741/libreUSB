/*
 * File: 	usb.c
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file implements a USB/CDC device firmware for PIC18F4550
 * microcontroller, it allows to create a virtual serial (COM) port for data
 * transmission with the PC over USB.
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


/*******************************************************************************
                              DEVICE CURRENT STATE

                 See USB 2.0 specification: page 241, table 9-1
*******************************************************************************/

volatile unsigned char USB_DEVICE_STATE;
volatile unsigned char USB_DEVICE_ADDRESS;
volatile unsigned char USB_DEVICE_CURRENT_CONFIGURATION;

/*******************************************************************************
*******************************************************************************/



/*******************************************************************************
                             USB handling functions

                       See PIC18F4550 datasheet: page 177
*******************************************************************************/

// General USB Handler
void usb_handler(void);

    // Interrupt handling
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
            static void handle_req_get_status(void);
            static void handle_req_clear_feature(void);
            static void handle_req_set_feature(void);
            static void handle_req_set_address(void);
            static void handle_req_get_descriptor(unsigned char
                    **descriptor, unsigned char *size);
            static void handle_req_get_configuration(void);
            static void handle_req_set_configuration(void);
            static void handle_req_get_interface(void);
            static void handle_req_set_interface(void);


/*******************************************************************************
*******************************************************************************/





/*******************************************************************************
                             ENDPOINT 0 definition

    Endpoint 0 out buffer starts at 0500h in data memory,
    is 8 bytes long [0500h - 0507h]

    Endpoint 0 out buffer starts at 0508h in data memory,
    is 64 bytes long [0508h - 056Bh]

                       See PIC18F4550 datasheet: page 170
*******************************************************************************/

// EP0 Buffer size in bytes
#define EP0_OUT_BUFFER_SIZE 8 // SetUp packet size
#define EP0_IN_BUFFER_SIZE 64

// Endpoint 0 buffers location
#define EP0_OUT_BUFFER  0x0500
#define EP0_IN_BUFFER  0x0508

// Endpoint 0 buffer descriptors allocation
volatile BUFFER_DESC_t __at(0x0400 + (0 * 8)) EP0_OUT;
volatile BUFFER_DESC_t __at(0x0404 + (0 * 8)) EP0_IN;

// Setup Packet is allocated in the out endpoint 0 buffer
volatile USB_SETUP_PACKET_t __at(EP0_OUT_BUFFER) SETUP_PACKET;


/*******************************************************************************
*******************************************************************************/





/*******************************************************************************
                             DESCRIPTORS definitions

    Describe a CDC/ACM device with 1 configuration only

    * Descriptors are placed into code memory so we can safe a lot of RAM

*******************************************************************************/


/*
 * DEVICE DESCRIPTOR
 *
 * This is a CDC Device class descriptor with one configuration only
*/
__code USB_DESC_DEVICE_t DEVICE_DESC =
{
    // bLength: Total device descriptor size
    sizeof(USB_DESC_DEVICE_t),

    // bDescriptorType: Device descriptor
    USB_DESC_TYPE_DEVICE,

    // bcdUSB: USB 2.0 compliant device
    USB_DEVICE_BCDUSB,

    // bDeviceClass: CDC device Class
    USB_CDC_CLASS_DEVICE,

    // bDeviceSubClass: No device SubClass
    0x00,

    // bDeviceProtocol: No device Protocol
    0x00,

    // bMaxPacketSize0: Max End Point 0 packet size is End Point 0 Buffer size
    EP0_OUT_BUFFER_SIZE,

    // idVendor: Using the "Microchip" Vendor ID
    0x04D8,

    // idProduct: Using arbitrary Product ID
    0x0111,

    // bcdDevice: No device version
    0x0000,

    // iManufacturer: Manufacturer description text
    0x01,

    // iProduct: Product description text
    0x02,

    // iSerialNumber: No serial number text
    0x00,

    // bNumConfigurations: This device has one configuration only
    0x01
};





/*
 * CONFIGURATION DESCRIPTOR
 *
 * This device has one configuration only
 *
 * This a USB/CDC (Communications Device Class) configuration
 *
 * The configuration descriptor should contain the complete interface, endpoint
 * and class specific descriptors hierarchy
 *
 * CONFIGURATION DESCRIPTOR hierarchy (USB/CDC):
 *     - Configuration Descriptor
 *         - Interface Descriptor (Communications)
 *             - Functional Descriptor (Header)
 *             - Functional Descriptor (ACM)
 *             - Functional Descriptor (Union)
 *             - Functional Descriptor (Call Management)
 *             - EndPoint Descriptor (Notification Element)
 *         - Interface Descriptor (Data)
 *             - EndPoint Descriptor (Data Out)
 *             - EndPoint Descriptor (Data In)
*/

struct
{
    USB_DESC_CONFIGURATION_t CONFIGURATION_DESC;

        USB_DESC_INTERFACE_t INTERFACE_DESC_COMMUNICATIONS;
            USB_CDC_DESC_FUNCTIONAL_HEADER_t FUNCTIONAL_DESC_HEADER;
            USB_CDC_DESC_FUNCTIONAL_ABSTRACT_CONTROL_MANAGEMENT_t FUNCTIONAL_DESC_ACM;
            USB_CDC_DESC_FUNCTIONAL_UNION_t FUNCTIONAL_DESC_UNION;
            USB_CDC_DESC_FUNCTIONAL_CALL_MANAGEMENT_t FUNCTIONAL_DESC_CALL_MANAGEMENT;
            USB_DESC_EP_t EP_DESC_NOTIFICATION_ELEMENT;

        USB_DESC_INTERFACE_t INTERFACE_DESC_DATA;
            USB_DESC_EP_t EP_DESC_OUT;
            USB_DESC_EP_t EP_DESC_IN;
}


/*
 * CONFIGURATION
 *
 * Contains whole configuration descriptors hierarchy
 */
__code CONFIGURATION_0 =
{
                         /* CONFIGURATION_DESCRIPTOR */
    {
    // bLength:             Configuration Descriptor size
    sizeof(USB_DESC_CONFIGURATION_t),

    // bDescriptorType:     Configuration descriptor
    USB_DESC_TYPE_CONFIGURATION,

    // wTotalLength:        Whole configuration hierarchy size
    sizeof(CONFIGURATION_0),

    // bNumInterfaces:      This configuration has 2 interfaces
    0x02,

    // bConfigurationValue: Index value for this configuration
    0x01,

    // iConfiguration:      No configuration description text
    0x00,

    // bmAttributes:        Bus Powered configuration
    USB_CONFIGURATION_BUSPOWERED,

    // bMaxPower:           This configuration takes up to 200mA from the bus
    USB_CONFIGURATION_MAXPOWER
    },



                   /* INTERFACE_DESCRIPTOR_COMMUNICATIONS */
    {
    // bLength: Interface Descriptor size
    sizeof(USB_DESC_INTERFACE_t),

    // bDescriptorType: Interface descriptor
    USB_DESC_TYPE_INTERFACE,

    // bInterfaceNumber: This is interface 0
    0x00,

    // bAlternateSetting: Alternate setting number
    0x00,

    // bNumEndpoints: This interface has 1 endpoint
    0x01,

    // bInterfaceClass: Communications interface class
    USB_CDC_CLASS_INTERFACE_COM,

    // bInterfaceSubClass: Abstract Control Model sub class
    USB_CDC_SUBCLASS_INTERFACE_ACM,

    // bInterfaceProtocol: V250 protocol
    USB_CDC_PROTOCOL_INTERFACE_V250,

    // iInterface: No interface description text
    0x00
    },



                       /* FUNCTIONAL_DESCRIPTOR_HEADER */
    {
    // bFunctionalLength: Header FD size
    sizeof(USB_CDC_DESC_FUNCTIONAL_HEADER_t),

    // bDescriptorType: Interface descriptor type
    USB_CDC_FUNCTIONAL_CS_INTERFACE,

    // bDescriptorSubType: Header FD
    USB_CDC_FUNCTIONAL_HEADER,

    // bcdCDC: USB CDC 1.2 compliant
    USB_CDC_HEADER_BCDUSB
    },



                        /* FUNCTIONAL_DESCRIPTOR_ACM */
    {
    // bFunctionalLength: ACM FD size
    sizeof(USB_CDC_DESC_FUNCTIONAL_ABSTRACT_CONTROL_MANAGEMENT_t),

    // bDescriptorType: Interface descriptor type
    USB_CDC_FUNCTIONAL_CS_INTERFACE,

    // bDescriptorSubType: ACM FD
    USB_CDC_FUNCTIONAL_ACM,

    // bmCapabilities: Support "line" requests
    USB_CDC_ACM_BMCAPABILITIES
    },



                       /* FUNCTIONAL_DESCRIPTOR_UNION */
    {
    // bFunctionalLength: Union FD size
    sizeof(USB_CDC_DESC_FUNCTIONAL_UNION_t),

    // bDescriptorType: Interface descriptor type
    USB_CDC_FUNCTIONAL_CS_INTERFACE,

    // bDescriptorSubType: Union FD
    USB_CDC_FUNCTIONAL_UNION,

    // bControlInterface: Interface 0 is the control interface
    // (communication class)
    0x00,

    // bSubordinateInterface0: Interface 1 is subordinate (data class)
    0x01
    },



                  /* FUNCTIONAL_DESCRIPTOR_CALL_MANAGEMENT */
    {
    // bFunctionalLength: CM FD size
    sizeof(USB_CDC_DESC_FUNCTIONAL_CALL_MANAGEMENT_t),

    // bDescriptorType: Interface descriptor type
    USB_CDC_FUNCTIONAL_CS_INTERFACE,

    // bDescriptorSubType: CM FD
    USB_CDC_FUNCTIONAL_CALL_MANAGEMENT,

    // bmCapabilities: Don't handle call management
    USB_CDC_CALL_MANAGEMENT_BMCAPABILITIES,

    // bDataInterface: Interface 1 is data class
    0x01
    },



                 /* ENDPOINT_DESCRIPTOR_NOTIFICATION_ELEMENT */
    {
    // bLength: Endpoint descriptor size
    sizeof(USB_DESC_EP_t),

    // bDescriptorType: Endpoint descriptor
    USB_DESC_TYPE_ENDPOINT,

    // bEndpointAddress: In endpoint 2
    USB_EP_02_IN,

    // bmAttributes: Interrupt endpoint
    USB_EP_INTERRUPT,

    // wMaxPacketSize: 64 bytes max packet
    0x40,

    // bInterval: Poll every 2 milliseconds
    0x02
    },



                        /* INTERFACE_DESCRIPTOR_DATA */
    {
    // bLength: Interface descriptor size
    sizeof(USB_DESC_INTERFACE_t),

    // bDescriptorType: Interface descriptor
    USB_DESC_TYPE_INTERFACE,

    // bInterfaceNumber: This is interface 1
    0x01,

    // bAlternateSetting: Alternate setting number
    0x00,

    // bNumEndpoints: This interface has 2 endpoints
    0x02,

    // bInterfaceClass: Data interface class
    USB_CDC_CLASS_INTERFACE_DAT,

    // bInterfaceSubClass: No interface class
    USB_CDC_SUBCLASS_INTERFACE_NONE,

    // bInterfaceProtocol: No interface protocol
    USB_CDC_PROTOCOL_INTERFACE_NONE,

    // iInterface: No interface description text
    0x00
    },



                         /* ENDPOINT_DESCRIPTOR_OUT */
    {
    // bLength: Endpoint descriptor size
    sizeof(USB_DESC_EP_t),

    // bDescriptorType: Endpoint descriptor
    USB_DESC_TYPE_ENDPOINT,

    // bEndpointAddress: Out endpoint 3
    USB_EP_03_OUT,

    // bmAttributes: Bulk endpoint
    USB_EP_BULK,

    // wMaxPacketSize: 64 bytes max packet
    USB_CDC_RX_BUFFER_SIZE,

    // bInterval: Poll as fast as possible
    0x00
    },



                          /* ENDPOINT_DESCRIPTOR_IN */
    {
    // bLength: Endpoint descriptor size
    sizeof(USB_DESC_EP_t),

    // bDescriptorType: Endpoint descriptor
    USB_DESC_TYPE_ENDPOINT,

    // bEndpointAddress: In endpoint 3
    USB_EP_03_IN,

    // bmAttributes: Bulk endpoint
    USB_EP_BULK,

    // wMaxPacketSize: 64 bytes max packet
    USB_CDC_TX_BUFFER_SIZE,

    // bInterval: Poll as fast as possible
    0x00
    }
};




/*
 * STRING DESCRIPTORS
 *
 * 2 bytes coding characters
 *
 * String descriptor 0 = Supported Languages
 * String descriptor 1 = iManufacturer
 * String descriptor 2 = iProduct
*/
__code unsigned char STRING_DESC_0[] =
{
    // bLength
    0x04,

    // bDescriptorType: String descriptor
    USB_DESC_TYPE_STRING,

    //wLANGID[0]: English (USB languages specification: page 5)
    0x09, 0x04
};


__code unsigned char STRING_DESC_1[] =
{
    // bLength
    0x24,

    // bDescriptorType: String descriptor
    USB_DESC_TYPE_STRING,

    // bString: "Silly-Bytes"
    'S', 0x00,
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


__code unsigned char STRING_DESC_2[] =
{
    // bLength
    0x34,

    // bDescriptorType: String descriptor
    USB_DESC_TYPE_STRING,

    // bString: "Virtual COM port"
    'V', 0x00,
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



/*******************************************************************************
*******************************************************************************/






/* Initializes the USB hardware */
void usb_init(void)
{
	// Set current device state
	USB_DEVICE_STATE = USB_STATE_DETACHED;
    USB_DEVICE_ADDRESS = 0x00;
    USB_DEVICE_CURRENT_CONFIGURATION = 0x00;

	// Clear all USB related registers
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

	// Enable USB module
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



/*
 * Handles USB requests, states and transactions
 *
 *  Always call this function using an IRQ like this:
 *
 *  void usb_isr(void) __shadowregs __interrupt 1
 *	{
 *		if( PIR2bits.USBIF )
 *		{
 *			usb_handler();
 *			PIR2bits.USBIF = 0;
 *		}
 *	}
 *
 */
void usb_handler(void)
{
    // If the device isn't in powered state avoid interrupt handling
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

	// An idle condition has been detected
	if( UIRbits.IDLEIF && UIEbits.IDLEIE )
    {
        // Do not suspend if the device is not addressed
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

	// An error condition has been detected
	if( UIRbits.UERRIF && UIEbits.UERRIE )
    {
        // Do nothing
        // TODO: I guess we should do something about this...

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

	// A reset signal has been achieved
	if( UIRbits.URSTIF && UIEbits.URSTIE )
    {
        handle_urstif();
        UIRbits.URSTIF = 0;
	}
}





              /***************  Interrupt Handlers  *************/

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
    // Clear interrupt flags
    UIR = 0x00;
    UEIR = 0x00;

    // Endpoint 0 configuration
    UEP0bits.EPINEN = 1; // Endpoint 0 IN enabled
    UEP0bits.EPOUTEN = 1; // Endpoint 0 OUT enabled
    UEP0bits.EPHSHK = 1; // Endpoint 0 handshake enabled
    UEP0bits.EPCONDIS = 0; // Endpoint 0 Control transfers allowed
    UEP0bits.EPSTALL = 0; // Endpoint 0 is not stalled

    // Flush transactions queue
    while( UIRbits.TRNIF ){ UIRbits.TRNIF = 0; }

    // Enable SIE packet processing
    UCONbits.PKTDIS = 0;


    // Configure endpoint 0 buffer descriptors so we're ready to
    // receive the first control transfer
    EP0_OUT.STAT.stat = 0x00; // Clear endpoint 0 buffer descriptors STAT
    EP0_IN.STAT.stat = 0x00;
    EP0_OUT.ADDR = EP0_OUT_BUFFER; // Buffer memory address for out ep0
    EP0_IN.ADDR = EP0_IN_BUFFER; // Buffer memory address for in ep0
    EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE; // Receive up to EP0_OUT_BUFFER_SIZE bytes
    EP0_OUT.STAT.UOWN = 1; // Give out buffer descriptor control to the SIE
    EP0_IN.STAT.UOWN = 0; // Give in buffer descriptor control to the CORE

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





              /***************  Transfers Handlers  *************/


/*
 * Handles control transfers
 *
 * Optional SET_DESCRIPTOR request not implemented here, so we don't need to
 * worry about any DATA OUT STAGE.
 *
 * Handle this 3 control transfer stages:
 *      - OUT direction transactions
 *          * SETUP stage
 *          * STATUS stage
 *      - IN direction transactions
 *          * DATA IN stage
*/
static void control_transfer_handler(void)
{
    // Pending data to send during the control transfer
    static unsigned char *data_to_send;

    // Pending number of bytes to send during the control transfer
    static unsigned char bytes_to_send;



    /*****  OUT direction transactions  (SETUP or STATUS stage)  *****/
	if (USTATbits.DIR == 0)
    {
        /*** SETUP transaction (SETUP stage) ***/
		if (EP0_OUT.STAT.PID == USB_PID_TOKEN_SETUP)
        {
            // The CPU owns the endpoint 0 buffer descriptors
            EP0_IN.STAT.UOWN = 0;
            EP0_OUT.STAT.UOWN = 0;


            /*** Handle requests ***/

            // GET_DESCRIPTOR request
            if ( SETUP_PACKET.bRequest == USB_REQ_GET_DESCRIPTOR )
            {
                handle_req_get_descriptor(&data_to_send, &bytes_to_send);

                // The descriptor fits in just one in transaction
                if( bytes_to_send <= SETUP_PACKET.wLength )
                {
                    // Fill IN buffer with the whole descriptor
                    int i=0;
                    for( i; i<bytes_to_send; i++)
                    {
                        *( (__data unsigned char*) EP0_IN_BUFFER + i ) =
                            *( (__code unsigned char*) data_to_send + i);
                    }


                    // Prepare OUT buffer
                    EP0_OUT.STAT.stat = 0x00;
                    EP0_OUT.ADDR = EP0_OUT_BUFFER;
                    EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE;

                    // Prepare IN buffer
                    EP0_IN.STAT.stat = 0x00;
                    EP0_IN.ADDR = (short)EP0_IN_BUFFER;
                    EP0_IN.CNT = (unsigned char) bytes_to_send;

                    // Update bytes_to_send, and data_to_send
                    // to 0 (no more data to send)
                    data_to_send = 0;
                    bytes_to_send = 0;

                    // Enable SIE packet processing
                    UCONbits.PKTDIS = 0;

                    // Give Buffer descriptors control to the SIE so the data
                    // can be sent
                    EP0_OUT.STAT.UOWN = 1;
                    EP0_IN.STAT.UOWN = 1;


                    return;
                }
                // The descriptor fits in multiple in transactions
                else
                {
                    // Fill IN buffer with the first descriptor bytes
                    int i=0;
                    for( i; i<SETUP_PACKET.wLength; i++)
                    {
                        *( (__data unsigned char*) EP0_IN_BUFFER + i ) =
                            *( data_to_send + i);
                    }

                    // Prepare OUT buffer
                    EP0_OUT.STAT.stat = 0x00;
                    EP0_OUT.ADDR = EP0_OUT_BUFFER;
                    EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE;

                    // Prepare IN buffer
                    EP0_IN.STAT.stat = 0x00;
                    EP0_IN.ADDR = EP0_IN_BUFFER;
                    EP0_IN.CNT = SETUP_PACKET.wLength;

                    // Update bytes_to_send, and data_to_send
                    data_to_send += SETUP_PACKET.wLength;
                    bytes_to_send -= SETUP_PACKET.wLength;

                    // Give Buffer descriptors control to the SIE so the data can be sent
                    EP0_OUT.STAT.UOWN = 1;
                    EP0_IN.STAT.UOWN = 1;

                    return;
                }
            }
        }
        /*** OUT transaction (STATUS stage) ***/
        else
        {
            /*
             * The host has confirmed the end of the control transfer
             * so we need to prepare everything for any future control transfer
            */

            // The CPU owns the endpoint 0 buffer descriptors
            // (so we can modify them)
            EP0_IN.STAT.UOWN = 0;
            EP0_OUT.STAT.UOWN = 0;

            // Configure endpoint 0 buffer descriptors so we're ready to
            // receive the future control transfer
            EP0_OUT.STAT.stat = 0x00;
            EP0_IN.STAT.stat = 0x00;
            EP0_OUT.ADDR = EP0_OUT_BUFFER;
            EP0_IN.ADDR = EP0_IN_BUFFER;
            EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE;
            EP0_OUT.STAT.UOWN = 1; // SIE controls OUT buffer
            EP0_IN.STAT.UOWN = 0; // CORE controls IN buffer

            return;
        }
    }
    /*****  IN direction transactions  (DATA IN stage)  *****/
    else
    {
        // The CPU owns the endpoint 0 buffer descriptors
        EP0_IN.STAT.UOWN = 0;
        EP0_OUT.STAT.UOWN = 0;

        // We have no more data to send, so send a 0 length packet to indicate
        // the end of the data in stage (USB 2.0 spec: page 253)
        if( bytes_to_send == 0 )
        {
            // Prepare OUT buffer
            EP0_OUT.STAT.stat = 0x00;
            EP0_OUT.ADDR = EP0_OUT_BUFFER;
            EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE;

            // Prepare IN buffer
            EP0_IN.STAT.stat = 0x00;
            EP0_IN.ADDR = EP0_IN_BUFFER;
            EP0_IN.CNT = 0; // 0 length packet

            // Enable SIE packet processing
            UCONbits.PKTDIS = 0;

            // Give Buffer descriptors control to the SIE so the data can be sent
            EP0_OUT.STAT.UOWN = 1;
            EP0_IN.STAT.UOWN = 1;

        }
        // No more data to send
        else
        {
            // The data fits in just one in transaction
            if( bytes_to_send <= EP0_IN_BUFFER_SIZE )
            {
                // Fill IN buffer with the complete data
                int i=0;
                for( i; i<bytes_to_send; i++)
                {
                    *( (__data unsigned char*) EP0_IN_BUFFER + i ) = *(
                            data_to_send + i);
                }

                // Prepare OUT buffer
                EP0_OUT.STAT.stat = 0x00;
                EP0_OUT.ADDR = EP0_OUT_BUFFER;
                EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE;

                // Prepare IN buffer
                EP0_IN.STAT.stat = 0x00;
                EP0_IN.ADDR = EP0_IN_BUFFER;
                EP0_IN.CNT = bytes_to_send;

                // Update bytes_to_send, and data_to_send
                // to 0 (no more data to send)
                data_to_send = 0;
                bytes_to_send = 0;

                // Give Buffer descriptors control to the SIE so the data can be sent
                EP0_OUT.STAT.UOWN = 1;
                EP0_IN.STAT.UOWN = 1;

                // Enable SIE packet processing
                UCONbits.PKTDIS = 0;

                return;
            }
            // The data fits in multiple in transactions
            else
            {
                // Fill IN buffer with the first data bytes
                int i=0;
                for( i; i<EP0_IN_BUFFER_SIZE; i++)
                {
                    *( (__data unsigned char*) EP0_IN_BUFFER + i ) = *( data_to_send + i);
                }

                // Prepare OUT buffer
                EP0_OUT.STAT.stat = 0x00;
                EP0_OUT.ADDR = EP0_OUT_BUFFER;
                EP0_OUT.CNT = EP0_OUT_BUFFER_SIZE;

                // Prepare IN buffer
                EP0_IN.STAT.stat = 0x00;
                EP0_IN.ADDR = EP0_IN_BUFFER;
                EP0_IN.CNT = EP0_IN_BUFFER_SIZE;

                // Update bytes_to_send, and data_to_send
                data_to_send += SETUP_PACKET.wLength;
                bytes_to_send -= SETUP_PACKET.wLength;

                // Give Buffer descriptors control to the SIE so the data can be sent
                EP0_OUT.STAT.UOWN = 1;
                EP0_IN.STAT.UOWN = 1;

                // Enable SIE packet processing
                UCONbits.PKTDIS = 0;

                return;
            }
        }
    }
}





              /***************  Requests Handlers  *************/


/*
 * Handle GET_DESCRIPTOR request
 *
 * DESCRIPTOR pointer will point to the first byte of the requested descriptor
 *
 * SIZE will contain the total size in bytes of the requested descriptor
*/
static void handle_req_get_descriptor(unsigned char **descriptor, unsigned char *size)
{
    // Descriptor type is the high byte of wValue field of the setup packet
    // (USB 2.0 spec: page 253)
    unsigned char descriptor_type = SETUP_PACKET.wValue1;

    // Descriptor index is the low byte of wValue field of the setup packet
    // (USB 2.0 spec: page 253)
    unsigned char descriptor_index = SETUP_PACKET.wValue0;



    // DEVICE DESCRIPTOR
    if( descriptor_type == USB_DESC_TYPE_DEVICE )
    {
        // Memory position of the descriptor
        *descriptor = (unsigned char*) &DEVICE_DESC;

        // Size of the descriptor
        *size = sizeof(DEVICE_DESC);

        return;
    }


    // CONFIGURATION DESCRIPTOR
    if( descriptor_type == USB_DESC_TYPE_CONFIGURATION )
    {
        // Memory position of the descriptor
        *descriptor = (unsigned char*) &CONFIGURATION_0;

        // Size of the descriptor
        *size = sizeof(CONFIGURATION_0);

        return;
    }


    // STRING DESCRIPTOR
    if( descriptor_type == USB_DESC_TYPE_STRING )
    {
        // String 0
        if( descriptor_index == 0 )
        {
            // Memory position of the descriptor
            *descriptor = (unsigned char*) &STRING_DESC_0;

            // Size of the descriptor
            *size = sizeof(STRING_DESC_0);
        }


        // String 1
        if( descriptor_index == 1 )
        {
            // Memory position of the descriptor
            *descriptor = (unsigned char*) &STRING_DESC_1;

            // Size of the descriptor
            *size = sizeof(STRING_DESC_1);
        }


        // String 2
        if( descriptor_index == 2 )
        {
            // Memory position of the descriptor
            *descriptor = (unsigned char*) &STRING_DESC_2;

            // Size of the descriptor
            *size = sizeof(STRING_DESC_2);
        }

        return;
    }
}
