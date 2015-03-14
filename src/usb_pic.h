/*
 * File: 	usb_pic.h
 * Compiler: sdcc (Version 3.4.0)
 *
 *
 * [!] This file contains USB definitions for PIC18F4550 microtroller described
 * by the datasheet
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


#ifndef _USB_PIC_H
#define _USB_PIC_H


/*******************************************************************************
                               BUFFER DESCRIPTOR

                 See PIC18F4550 datasheet: page 176 table 17-5
*******************************************************************************/


typedef struct
{
	union
	{
		struct // CPU MODE
		{
			unsigned BC8	:1;
			unsigned BC9	:1;
			unsigned BSTALL	:1;
			unsigned DTSEN	:1;
			unsigned INCDIS	:1;
			unsigned KEN 	:1;
			unsigned DTS	:1;
			unsigned UOWN	:1;
		};

		struct // SIE MODE
		{
			unsigned 		:2;
			unsigned PID0	:1;
			unsigned PID1	:1;
			unsigned PID2	:1;
			unsigned PID3 	:1;
			unsigned 		:2;
		};

		struct // SIE MODE
		{
			unsigned  		:2;
			unsigned PID 	:4;
			unsigned 		:2;
		};

        unsigned char stat; // Whole STAT byte

	} STAT;

	unsigned char CNT;
	unsigned short ADDR;

} BUFFER_DESCRIPTOR_t;



/*
 * ----------------------------------------------------------------
 *                       BUFFER DESCRIPTORS
 *
 * See PIC18F4550 datasheet: page 175 figure 17-7
 *
 * - Endpoints buffer descriptors are allocated after 400h
 * - Each BD take 4 bytes
 *
 * Memory position formula: (base_direction + endpoint_number * 8)
 *
 * Base direction: (OUT endpoint = 400), (IN endpoint = 404)
 *
 *------------------------------------------------------------------
 */

// Endpoint 0 buffer descriptors
extern volatile BUFFER_DESCRIPTOR_t __at(0x0400 + (0 * 8)) ENDPOINT0_OUT;
extern volatile BUFFER_DESCRIPTOR_t __at(0x0404 + (0 * 8)) ENDPOINT0_IN;

// Unused Endpoint 1-3 buffer descriptors
//extern volatile BUFFER_DESCRIPTOR_t __at(0x0400 + 1 * 8) ENDPOINT1_OUT;
//extern volatile BUFFER_DESCRIPTOR_t __at(0x0404 + 1 * 8) ENDPOINT1_IN;
//extern volatile BUFFER_DESCRIPTOR_t __at(0x0400 + 2 * 8) ENDPOINT2_OUT;
//extern volatile BUFFER_DESCRIPTOR_t __at(0x0404 + 2 * 8) ENDPOINT2_IN;
//extern volatile BUFFER_DESCRIPTOR_t __at(0x0400 + 3 * 8) ENDPOINT3_OUT;
//extern volatile BUFFER_DESCRIPTOR_t __at(0x0404 + 3 * 8) ENDPOINT3_IN;

/*******************************************************************************
*******************************************************************************/


#endif // _USB_PIC_H
