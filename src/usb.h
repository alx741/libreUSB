/*
 * File: 	usb.h
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file cotains USB definitions described by
 * 	   usb 2.0 specification at usb.org
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


#ifndef _USB_H
#define _USB_H



/************************************************
 * **********************************************
 *
 * STATE
 * (usb 2.0 specification, page 241, table 9-1)
 *
 * **********************************************
 ************************************************/

// DEVICE CURRENT STATE
extern volatile unsigned char USB_DEVICE_STATE;
extern volatile unsigned char USB_DEVICE_ADDRESS;
extern volatile unsigned char USB_DEVICE_CURRENT_CONFIGURATION;

// STATES
#define USB_STATE_DETACHED 0x00
#define USB_STATE_ATTACHED 0x01
#define USB_STATE_POWERED 0x02
#define USB_STATE_DEFAULT 0x03
#define USB_STATE_ADDRESS 0x04
#define USB_STATE_CONFIGURED 0x05
#define USB_STATE_SUSPENDED 0x06



/************************************************
 * **********************************************
 *
 * PID values
 * (usb 2.0 specification, page 196, table 8-1)
 *
 * **********************************************
 ************************************************/

// Token PID's
#define USB_PID_TOKEN_OUT 0x01
#define USB_PID_TOKEN_IN 0x09
#define USB_PID_TOKEN_SOF 0x05
#define USB_PID_TOKEN_SETUP 0x0D

// Data PID's
#define USB_PID_DATA_DATA0 0x03
#define USB_PID_DATA_DATA1 0x0B
#define USB_PID_DATA_DATA2 0x07
#define USB_PID_DATA_MDATA 0x0F

// Handshake PID's
#define USB_PID_HANDSHAKE_ACK 0x02
#define USB_PID_HANDSHAKE_NAK 0x0A
#define USB_PID_HANDSHAKE_STALL 0x0E
#define USB_PID_HANDSHAKE_NYET 0x06

// Special PID's
#define USB_PID_SPECIAL_PRE 0x0C
#define USB_PID_SPECIAL_ERR 0x0C
#define USB_PID_SPECIAL_SPLIT 0x08
#define USB_PID_SPECIAL_PING 0x04



/************************************************
 * **********************************************
 *
 * REQUESTS
 * (usb 2.0 specification, page 251, table 9-4)
 *
 * **********************************************
 ************************************************/

#define USB_REQUEST_GET_STATUS 0x00
#define USB_REQUEST_CLEAR_FEATURE 0x01
#define USB_REQUEST_SET_FEATURE 0x03
#define USB_REQUEST_SET_ADDRESS 0x05
#define USB_REQUEST_GET_DESCRIPTOR 0x06
#define USB_REQUEST_SET_DESCRIPTOR 0x07
#define USB_REQUEST_GET_CONFIGURATION 0x08
#define USB_REQUEST_SET_CONFIGURATION 0x09
#define USB_REQUEST_GET_INTERFACE 0x0A
#define USB_REQUEST_SET_INTERFACE 0x0B
#define USB_REQUEST_SYNCH_FRAME 0x0C



/************************************************
 * **********************************************
 *
 * SETUP PACKET
 * (usb 2.0 specification, page 248, table 9-2)
 *
 * **********************************************
 ************************************************/

typedef struct
{
	unsigned char bmRequestType;
	unsigned char bRequest;
	unsigned short wValue;
	unsigned short wIndex;
	unsigned short wLength;
} USB_SETUP_PACKET_t;



/************************************************
 * **********************************************
 *
 * DESCRIPTORS
 * (usb 2.0 specification, page 251, table 9-5)
 *
 * **********************************************
 ************************************************/

// DESCRIPTOR TYPES
// Field: bDescriptorType
#define USB_DESCRIPTOR_TYPE_DEVICE 0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION 0x02
#define USB_DESCRIPTOR_TYPE_STRING 0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT 0x05


/* ---------------------------------------------
 * DEVICE DESCRIPTOR
 * (usb 2.0 specification, page 262, table 9-8)
 * ---------------------------------------------*/

// Field: bcdUSB
#define USB_DEVICE_BCDUSB 0x0200 // USB 2.0 compliant device


typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short bcdUSB;
	unsigned char bDeviceClass;
	unsigned char bDeviceSubClass;
	unsigned char bDeviceProtocol;
	unsigned char bMaxPacketSize0;
	unsigned short idVendor;
	unsigned short idProduct;
	unsigned short bcdDevice;
	unsigned char iManufacturer;
	unsigned char iProduct;
	unsigned char iSerialNumber;
	unsigned char bNumConfigurations;
} USB_DESCRIPTOR_DEVICE_t;


/* ---------------------------------------------
 * CONFIGURATION DESCRIPTOR
 * (usb 2.0 specification, page 265, table 9-10)
 * ---------------------------------------------*/

// Field: bmAttributes
// This 2 attributes should be ORed 
#define USB_CONFIGURATION_SELFPOWERED 0x40 // Self-Powered
#define USB_CONFIGURATION_REMOTEWAKEUP 0x20 // Remote wake up


typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short wTotalLength;
	unsigned char bNumInterfaces;
	unsigned char bConfigurationValue;
	unsigned char iConfiguration;
	unsigned char bmAttributes;
	unsigned char bMaxPower;
} USB_DESCRIPTOR_CONFIGURATION_t;


/* ---------------------------------------------
 * INTERFACE DESCRIPTOR
 * (usb 2.0 specification, page 268, table 9-12)
 * ---------------------------------------------*/

typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bInterfaceNumber;
	unsigned char bAlternateSetting;
	unsigned char bNumEndpoints;
	unsigned char bInterfaceClass;
	unsigned char bInterfaceSubClass;
	unsigned char bInterfaceProtocol;
	unsigned char iInterface;
} USB_DESCRIPTOR_INTERFACE_t;


/* ---------------------------------------------
 * ENDPOINT DESCRIPTOR
 * (usb 2.0 specification, page 269, table 9-13)
 * ---------------------------------------------*/

// Field: bEndpointAddress
#define USB_ENDPOINT_00_OUT 0x00
#define USB_ENDPOINT_00_IN 0x80
#define USB_ENDPOINT_01_OUT 0x01
#define USB_ENDPOINT_01_IN 0x81
#define USB_ENDPOINT_02_OUT 0x02
#define USB_ENDPOINT_02_IN 0x82
#define USB_ENDPOINT_03_OUT 0x03
#define USB_ENDPOINT_03_IN 0x83
#define USB_ENDPOINT_04_OUT 0x04
#define USB_ENDPOINT_04_IN 0x84
#define USB_ENDPOINT_05_OUT 0x05
#define USB_ENDPOINT_05_IN 0x85
#define USB_ENDPOINT_06_OUT 0x06
#define USB_ENDPOINT_06_IN 0x86
#define USB_ENDPOINT_07_OUT 0x07
#define USB_ENDPOINT_07_IN 0x87
#define USB_ENDPOINT_08_OUT 0x08
#define USB_ENDPOINT_08_IN 0x88
#define USB_ENDPOINT_09_OUT 0x09
#define USB_ENDPOINT_09_IN 0x89
#define USB_ENDPOINT_10_OUT 0x0A
#define USB_ENDPOINT_10_IN 0x8A
#define USB_ENDPOINT_11_OUT 0x0B
#define USB_ENDPOINT_11_IN 0x8B
#define USB_ENDPOINT_12_OUT 0x0C
#define USB_ENDPOINT_12_IN 0x8C
#define USB_ENDPOINT_13_OUT 0x0D
#define USB_ENDPOINT_13_IN 0x8D
#define USB_ENDPOINT_14_OUT 0x0E
#define USB_ENDPOINT_14_IN 0x8E
#define USB_ENDPOINT_15_OUT 0x0F
#define USB_ENDPOINT_15_IN 0x8F

// Field: bmAttributes
// This 11 attributes should be ORed 
#define USB_ENDPOINT_CONTROL 0x00 // Control transfer endpoint
#define USB_ENDPOINT_ISOCHRONOUS 0x01 // Isochronous transfer endpoint
#define USB_ENDPOINT_BULK 0x02 // Bulk transfer endpoint
#define USB_ENDPOINT_INTERRUPT 0x03 // Interrupt transfer endpoint
#define USB_ENDPOINT_NO_SYNCHRONIZATION 0x00 // No synchronization (Isochronous endpoints only)
#define USB_ENDPOINT_ASYNCHRONOUS 0x04 // Asynchronous (Isochronous endpoints only)
#define USB_ENDPOINT_ADAPTIVE 0x08 // Adaptive (Isochronous endpoints only)
#define USB_ENDPOINT_SYNCHRONOUS 0x12 // Synchronous (Isochronous endpoints only)
#define USB_ENDPOINT_DATA 0x00 // Data endpoint
#define USB_ENDPOINT_FEEDBACK 0x10 // Feedback endpoint
#define USB_ENDPOINT_IMPLICIT_FEEDBACK_DATA 0x20 // Implicit feedback data endpoint


typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
	unsigned short wMaxPacketSize;
	unsigned char bInterval;
} USB_DESCRIPTOR_ENDPOINT_t;


#endif // _USB_H
