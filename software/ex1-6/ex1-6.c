#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define Bluetooth_Control     (*(volatile unsigned char *)(0x84000220))
#define Bluetooth_Status      (*(volatile unsigned char *)(0x84000220))
#define Bluetooth_TxData      (*(volatile unsigned char *)(0x84000222))
#define Bluetooth_RxData      (*(volatile unsigned char *)(0x84000222))
#define Bluetooth_Baud        (*(volatile unsigned char *)(0x84000224))

#define command_seq "$$$"
#define data_seq  "---"

void init_Bluetooth(void);
char putcharBluetooth(char c);
char getcharBluetooth(void);
int BluetoothTestForTransmitData(void);
int BluetoothTestForReceivedData(void);

void enter_command_mode(void);
void enter_data_mode(void);
void set_device_name(void);

int main(void) {

	//Initialize the bluetooth
	init_Bluetooth();

	//Set the device name to group14
	enter_command_mode();
	set_device_name();

	//Connect to bluetooth with Android phone

	while(1){

	}

	return 0;
}

void enter_command_mode(void){

	//Send the "$$$" to force into command mode
	putcharBluetooth('$');
	putcharBluetooth('$');
	putcharBluetooth('$');

	//Delay for 1 second
	usleep(1050);
}

void enter_data_mode(void){
	//Send the "$$$" to force into command mode
	putcharBluetooth('-');
	putcharBluetooth('-');
	putcharBluetooth('-');
}

void set_device_name(void){
	char* name_command = "SN,Group14\r\n";
	int command_length = strlen(name_command);

	int index;
	for(index = 0; index < command_length; index++){
		putcharBluetooth(name_command[index]);
	}
}

/*
 *  Subroutine to initialize the Bluetooth Port by writing some data
 *  to the internal registers.
 *  Call this function at the start of the program before you attempt
 *  to read or write to data via the Bluetooth port
 *  Refer to 6850 data sheet for details of registers
 *
 */
void init_Bluetooth(void) {
	// set up 6850 Control Register to utilize a divide by 16 clock,
	// set RTS low, use 8 bits of data, no parity, 1 stop bit,
	// transmitter interrupt disabled
	// Bluetooth_Control(7 DOWNTO 0) = |X|0|0|1|0|1|0|1| = 0b00010101 = 0x15
	// Bluetooth_Control(7 DOWNTO 0) = |X|1|0|1|0|1|0|1| = 0b01010101 = 0x55
	Bluetooth_Control 	= 0x15;

	// program baud rate generator to use 115k baud
	Bluetooth_Baud		= 0x01;
}

char putcharBluetooth(char c) {
	// poll Tx bit in 6850 status register. Wait for it to become '1'
	while(BluetoothTestForTransmitData() == 0);

	// write 'c' to the 6850 TxData register to output the character
	Bluetooth_TxData = c;

	return c; // return c
}

char getcharBluetooth(void) {
	// poll Rx bit in 6850 status register. Wait for it to become '1'
	while(BluetoothTestForReceivedData() == 0);

	// read received character from 6850 RxData register.

	char res = Bluetooth_RxData;

	return res;
}

// the following function polls the 6850 to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read
int BluetoothTestForReceivedData(void) {
	// Test Rx bit in 6850 serial communications chip status register
	// if RX bit is set, return TRUE, otherwise return FALSE

	// Bluetooth_Status: XXXX XXXX
	// We want bit0: 0000 0001
	if ((Bluetooth_Status & 0x1) == 0x1)
		return 1;
	else
		return 0;
}

int BluetoothTestForTransmitData(void) {
	// Test Tx bit in 6850 serial communications chip status register
	// if TX bit is set, return TRUE, otherwise return FALSE

	// Bluetooth_Status: XXXX XXXX
	// We want bit1: 0000 0010
	if ((Bluetooth_Status & 0x2) == 0x2)
		return 1;
	else
		return 0;
}
