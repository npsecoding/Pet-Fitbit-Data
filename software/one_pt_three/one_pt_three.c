// Go through Lecture 3 - Nios II Interfacing - Part 1 slides 14-15

#include <stdio.h>
#include <stdlib.h>

#define RS232_Control     (*(volatile unsigned char *)(0x84000200))
#define RS232_Status      (*(volatile unsigned char *)(0x84000200))
#define RS232_TxData      (*(volatile unsigned char *)(0x84000202))
#define RS232_RxData      (*(volatile unsigned char *)(0x84000202))
#define RS232_Baud        (*(volatile unsigned char *)(0x84000204))

#define arr_sz 5
#define DEBUG 0

void init_RS232(void);
int putcharRS232(int c);
int getcharRS232(void);
int RS232TestForTransmitData(void);
int RS232TestForReceivedData(void);

int main(void) {
	printf("test starting...\n\n");

	int input[arr_sz] = {0x1, 0x2, 0x3, 0x4, 0x5};
	int output[arr_sz];
	int i;

	init_RS232();

	for(i = 0; i < arr_sz; i++) {
		putcharRS232(input[i]);
		output[i] = getcharRS232();
	}

	for(i = 0; i < arr_sz; i++) {
		if (output[i] != input[i]) {
			printf("test failed!\n");
			return EXIT_FAILURE;
		}
	}

	printf("test passed!\n");
	return EXIT_SUCCESS;
}

/*
 *  Subroutine to initialize the RS232 Port by writing some data
 *  to the internal registers.
 *  Call this function at the start of the program before you attempt
 *  to read or write to data via the RS232 port
 *  Refer to 6850 data sheet for details of registers
 *
 */
void init_RS232(void) {
	if (DEBUG == 0) printf("initializing rs232...\n\n");

	// set up 6850 Control Register to utilize a divide by 16 clock,
	// set RTS low, use 8 bits of data, no parity, 1 stop bit,
	// transmitter interrupt disabled
	// RS232_Control(7 DOWNTO 0) = |X|0|0|1|0|1|0|1| = 0b00010101 = 0x15
	// RS232_Control(7 DOWNTO 0) = |X|1|0|1|0|1|0|1| = 0b01010101 = 0x55
	RS232_Control 	= 0x15;
	
	// program baud rate generator to use 115k baud
	RS232_Baud		= 0x01;
}

int putcharRS232(int c) {
	if (DEBUG == 0) printf("polling Tx bit...\n");

	// poll Tx bit in 6850 status register. Wait for it to become '1'
	while(RS232TestForTransmitData() == 0);

	// write 'c' to the 6850 TxData register to output the character
	if (DEBUG == 0) printf("writing '%d' to the 6850 TxData register...\n\n", c);
	RS232_TxData = c;

	return c; // return c
}

int getcharRS232(void) {
	if (DEBUG == 0) printf("polling Rx bit...\n");

	// poll Rx bit in 6850 status register. Wait for it to become '1'
	while(RS232TestForReceivedData() == 0);

	// read received character from 6850 RxData register.

	int res = RS232_RxData;
	if (DEBUG == 0) printf("received character from 6850 RxData register is %d...\n\n", res);

	return res;
}

// the following function polls the 6850 to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read
int RS232TestForReceivedData(void) {
	// Test Rx bit in 6850 serial communications chip status register
	// if RX bit is set, return TRUE, otherwise return FALSE

	// RS232_Status: XXXX XXXX
	// We want bit0: 0000 0001
	if ((RS232_Status & 0x1) == 0x1)
		return 1;
	else
		return 0;
}

int RS232TestForTransmitData(void) {
	// Test Tx bit in 6850 serial communications chip status register
	// if TX bit is set, return TRUE, otherwise return FALSE

	// RS232_Status: XXXX XXXX
	// We want bit1: 0000 0010
	if ((RS232_Status & 0x2) == 0x2)
		return 1;
	else
		return 0;
}
