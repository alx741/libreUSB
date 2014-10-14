/*
 * File: 	usb_cdc.h
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file cotains USB definitions described by
 * 	   usb 2.0, [CDC] Communications device class specification 
 * 	   and [ACM/PSTN] device subclass specification at usb.org
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


#ifndef _USB_CDC_H
#define _USB_CDC_H


/* #######################################################################
 * ######################     INTERFACE     ##############################
 * #######################################################################
 */

// DEFINITIONS
#define USB_CDC_RX_BUFFER_SIZE 64 // Data interface bulk out endpoint
#define USB_CDC_TX_BUFFER_SIZE 64 // Data interface bulk in endpoint

// CDC BUFFERS
extern volatile unsigned char USB_CDC_RX_BUFFER[CDC_RX_BUFFER_SIZE];
extern volatile unsigned char USB_CDC_TX_BUFFER[CDC_TX_BUFFER_SIZE];


// FUNCTIONS

/* Sends a character C to CDC_TX_BUFFER
 *
 * Returns a non-zero value if success
 */
int usb_cdc_putc(char c);

/* Returns a character from CDC_RX_BUFFER
 *
 * Block until a character has been received
 */
char usb_cdc_getc(void);

/* Sends a character string STR to CDC_TX_BUFFER
 *
 * Returns a non-zero value if success
 */
int usb_cdc_puts(char *str);

/* Reads a character string STR from CDC_RX_BUFFER
 *
 * Returns a non-zero value if success
 */
int usb_cdc_gets(char *str);


/* #######################################################################
 * #######################################################################
 * #######################################################################
 */



/************************************************
 * **********************************************
 *
 * CDC DEVICE
 * (usb 2.0, cdc specification, page 12, table 2)
 *
 * **********************************************
 ************************************************/

#define USB_CDC_CLASS_DEVICE 0x02 // Communications device class



/************************************************
 * **********************************************
 *
 * CDC/ACM REQUESTS
 * (usb 2.0, pstn specification, page 19, table 11)
 *
 * **********************************************
 ************************************************/

#define USB_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND 0x00
#define USB_CDC_REQUEST_GET_ENCAPSULATED_RESPONSE 0x01
#define USB_CDC_REQUEST_SET_COMM_FEATURE 0x02
#define USB_CDC_REQUEST_GET_COMM_FEATURE 0x03
#define USB_CDC_REQUEST_CLEAR_COMM_FEATURE 0x04
#define USB_CDC_REQUEST_SET_LINE_CODING 0x20
#define USB_CDC_REQUEST_GET_LINE_CODING 0x21
#define USB_CDC_REQUEST_SET_CONTROL_LINE_STATE 0x22
#define USB_CDC_REQUEST_SEND_BREAK 0x23



/************************************************
 * **********************************************
 *
 * CDC/ACM NOTIFICATIONS
 * (usb 2.0, pstn specification, page 31, table 28)
 *
 * **********************************************
 ************************************************/

#define USB_CDC_NOTIFICATION_NETWORK_CONNECTION 0x00
#define USB_CDC_NOTIFICATION_RESPONSE_AVAILABLE 0x01
#define USB_CDC_NOTIFICATION_SERIAL_STATE 0x20



/************************************************
 * **********************************************
 *
 * COMMUNICATIONS INTERFACE
 * (usb 2.0, cdc specification, page 12,
 * table 3,4,5)
 *
 * **********************************************
 ************************************************/

// Field: bInterfaceClass
#define USB_CDC_CLASS_INTERFACE_COM 0x02 // Communications interface class

// Field: bInterfaceSubClass
#define USB_CDC_SUBCLASS_INTERFACE_ACM 0x02 // Abstract control model interface subclass

// Field: bInterfaceProtocol
#define USB_CDC_PROTOCOL_INTERFACE_V250 0x01 // At commands: v250 interface protocol



/************************************************
 * **********************************************
 *
 * DATA INTERFACE
 * (usb 2.0, cdc specification, page 13,
 * table 6)
 *
 * **********************************************
 ************************************************/

// Field: bInterfaceClass
#define USB_CDC_CLASS_INTERFACE_DAT 0x0A // Data interface class

// Field: bInterfaceSubClass
#define USB_CDC_SUBCLASS_INTERFACE_NONE 0x00 // No interface subclass

// Field: bInterfaceProtocol
#define USB_CDC_PROTOCOL_INTERFACE_NONE 0x00 // No interface protocol



/************************************************
 * **********************************************
 *
 * FUNCTIONAL DESCRIPTORS
 * (usb 2.0, cdc specification, page 16,
 * table 11, 12, 13, 14)
 *
 * (usb 2.0, pstn specification, page 16,
 * table 11, 12, 13, 14)
 *
 * **********************************************
 ************************************************/

// FUNCTIONAL DESCRIPTOR TYPES
// Field: bDescriptorType
#define USB_CDC_FUNCTIONAL_CS_INTERFACE 0x24 // Interface functional descriptor
#define USB_CDC_FUNCTIONAL_CS_ENDPOINT 0x25 // Endpoint functional descriptor

// FUNCTIONAL DESCRIPTOR SUBTYPES
// Field: bDescriptorSubType
#define USB_CDC_FUNCTIONAL_HEADER 0x00 // Header functional descriptor
#define USB_CDC_FUNCTIONAL_CALL_MANAGEMENT 0x01 // Call management functional descriptor
#define USB_CDC_FUNCTIONAL_ACM 0x02 // Abstract control management functional descriptor
#define USB_CDC_FUNCTIONAL_UNION 0x06 // Union functional descriptor


/* ---------------------------------------------
 * HEADER FUNCTIONAL DESCRIPTOR
 * (usb 2.0, cdc specification, page 18, table 15)
 * ---------------------------------------------*/

// Field: bcdCDC
#define USB_CDC_HEADER_BCDUSB 0x0110 // USB CDC 1.2 compliant device


typedef struct
{
	unsigned char bFunctionalLength;
	unsigned char bDescriptorType;
	unsigned char bDescriptorSubType;
	unsigned short bcdCDC;
} USB_CDC_DESCRIPTOR_FUNCTIONAL_HEADER_t;


/* ---------------------------------------------
 * UNION FUNCTIONAL DESCRIPTOR
 * (usb 2.0, cdc specification, page 19, table 16)
 * ---------------------------------------------*/

typedef struct
{
	unsigned char bFunctionalLength;
	unsigned char bDescriptorType;
	unsigned char bDescriptorSubType;
	unsigned char bControlInterface;
	unsigned char bSubordinateInterface0;
} USB_CDC_DESCRIPTOR_FUNCTIONAL_UNION_t;


/* ---------------------------------------------
 * CALL MANAGEMENT FUNCTIONAL DESCRIPTOR
 * (usb 2.0, pstn specification, page 11, table 3)
 * ---------------------------------------------*/

typedef struct
{
	unsigned char bFunctionalLength;
	unsigned char bDescriptorType;
	unsigned char bDescriptorSubType;
	unsigned char bmCapabilities;
	unsigned char bDataInterface;
} USB_CDC_DESCRIPTOR_FUNCTIONAL_CALL_MANAGEMENT_t;


/* ---------------------------------------------
 * ABSTRACT CONTROL MANAGEMENT FUNCTIONAL DESCRIPTOR
 * (usb 2.0, pstn specification, page 12, table 4)
 * ---------------------------------------------*/

typedef struct
{
	unsigned char bFunctionalLength;
	unsigned char bDescriptorType;
	unsigned char bDescriptorSubType;
	unsigned char bmCapabilities;
} USB_CDC_DESCRIPTOR_FUNCTIONAL_ABSTRACT_CONTROL_MANAGEMENT_t;

#endif // _USB_CDC_H
