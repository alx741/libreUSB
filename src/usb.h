/*
 * File: 	usb.h
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file contains USB definitions described by USB 2.0 specification
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



/*******************************************************************************
                                   USB STATE

                 See USB 2.0 specification: page 241 table 9-1
*******************************************************************************/

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

/*******************************************************************************
*******************************************************************************/



/*******************************************************************************
                                   PID VALUES

                 See USB 2.0 specification: page 196 table 8-1
*******************************************************************************/

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

/*******************************************************************************
*******************************************************************************/



/*******************************************************************************
                                    REQUESTS

                 See USB 2.0 specification: page 251 table 9-4
*******************************************************************************/

#define USB_REQ_GET_STATUS 0x00
#define USB_REQ_CLEAR_FEATURE 0x01
#define USB_REQ_SET_FEATURE 0x03
#define USB_REQ_SET_ADDRESS 0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_DESCRIPTOR 0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE 0x0A
#define USB_REQ_SET_INTERFACE 0x0B
#define USB_REQ_SYNCH_FRAME 0x0C

/*******************************************************************************
*******************************************************************************/



/*******************************************************************************
                                  SETUP PACKET

                 See USB 2.0 specification: page 248 table 9-2
*******************************************************************************/

typedef struct
{
	unsigned char bmRequestType;
	unsigned char bRequest;
	unsigned char wValue0; // Low byte
	unsigned char wValue1; // High byte
	unsigned char wIndex0; // Low byte
	unsigned char wIndex1; // High byte
	unsigned short wLength;
} USB_SETUP_PACKET_t;

/*******************************************************************************
*******************************************************************************/



/*******************************************************************************
                                  DESCRIPTORS

                 See USB 2.0 specification: page 251 table 9-5
*******************************************************************************/

// Descriptor types used in field: bDescriptorType
#define USB_DESC_TYPE_DEVICE 0x01
#define USB_DESC_TYPE_CONFIGURATION 0x02
#define USB_DESC_TYPE_STRING 0x03
#define USB_DESC_TYPE_INTERFACE 0x04
#define USB_DESC_TYPE_ENDPOINT 0x05



/*
 * ---------------------------------------------
 *              DEVICE DESCRIPTOR
 * See USB 2.0 specification: page 262 table 9-8
 * ---------------------------------------------
 */

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
} USB_DESC_DEVICE_t;



/*
 * ----------------------------------------------
 *          CONFIGURATION DESCRIPTOR
 * See USB 2.0 specification: page 265 table 9-10
 * ----------------------------------------------
 */

// Field: bmAttributes
// This attributes should be logical ORed
#define USB_CONFIGURATION_BUSPOWERED 0x80
#define USB_CONFIGURATION_REMOTEWAKEUP 0x20
#define USB_CONFIGURATION_MAXPOWER 0x64 // 100 * (2mA units) = 200mA


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
} USB_DESC_CONFIGURATION_t;



/*
 * ----------------------------------------------
 *              INTERFACE DESCRIPTOR
 * See USB 2.0 specification: page 268 table 9-12
 * ----------------------------------------------
 */

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
} USB_DESC_INTERFACE_t;



/*
 * ----------------------------------------------
 *              ENDPOINT DESCRIPTOR
 * See USB 2.0 specification: page 269 table 9-13
 * ----------------------------------------------
 */

// Field: bEndpointAddress
#define USB_EP_00_OUT 0x00
#define USB_EP_00_IN 0x80
#define USB_EP_01_OUT 0x01
#define USB_EP_01_IN 0x81
#define USB_EP_02_OUT 0x02
#define USB_EP_02_IN 0x82
#define USB_EP_03_OUT 0x03
#define USB_EP_03_IN 0x83
#define USB_EP_04_OUT 0x04
#define USB_EP_04_IN 0x84
#define USB_EP_05_OUT 0x05
#define USB_EP_05_IN 0x85
#define USB_EP_06_OUT 0x06
#define USB_EP_06_IN 0x86
#define USB_EP_07_OUT 0x07
#define USB_EP_07_IN 0x87
#define USB_EP_08_OUT 0x08
#define USB_EP_08_IN 0x88
#define USB_EP_09_OUT 0x09
#define USB_EP_09_IN 0x89
#define USB_EP_10_OUT 0x0A
#define USB_EP_10_IN 0x8A
#define USB_EP_11_OUT 0x0B
#define USB_EP_11_IN 0x8B
#define USB_EP_12_OUT 0x0C
#define USB_EP_12_IN 0x8C
#define USB_EP_13_OUT 0x0D
#define USB_EP_13_IN 0x8D
#define USB_EP_14_OUT 0x0E
#define USB_EP_14_IN 0x8E
#define USB_EP_15_OUT 0x0F
#define USB_EP_15_IN 0x8F

// Field: bmAttributes
// This attributes should be logical ORed
#define USB_EP_CONTROL 0x00
#define USB_EP_ISOCHRONOUS 0x01
#define USB_EP_BULK 0x02
#define USB_EP_INTERRUPT 0x03
#define USB_EP_NO_SYNCHRONIZATION 0x00
#define USB_EP_ASYNCHRONOUS 0x04
#define USB_EP_ADAPTIVE 0x08
#define USB_EP_SYNCHRONOUS 0x12
#define USB_EP_DATA 0x00
#define USB_EP_FEEDBACK 0x10
#define USB_EP_IMPLICIT_FEEDBACK_DATA 0x20


typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
	unsigned short wMaxPacketSize;
	unsigned char bInterval;
} USB_DESC_EP_t;

/*******************************************************************************
*******************************************************************************/


#endif // _USB_H
