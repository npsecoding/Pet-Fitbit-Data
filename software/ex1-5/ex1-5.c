#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "altera_up_avalon_character_lcd.h"

#ifndef NULL
#define NULL   ((void *) 0)
#endif

#define DEBUG 1

#define GPS_Control     (*(volatile unsigned char *)(0x84000210))
#define GPS_Status      (*(volatile unsigned char *)(0x84000210))
#define GPS_TxData      (*(volatile unsigned char *)(0x84000212))
#define GPS_RxData      (*(volatile unsigned char *)(0x84000212))
#define GPS_Baud        (*(volatile unsigned char *)(0x84000214))

//FOr testing
#define delay_time 10000000

struct gps_data_entry{
	  char  utc_time[11];
	  char  latitude[10];
	  char  longitude[11];
	  char  n_s_indicator[2];
	  char  e_w_indicator[2];
	  int   used;
};

void init_GPS(void);
char putcharGPS(char c);
char getcharGPS(void);
int GPSTestForTransmitData(void);
int GPSTestForReceivedData(void);

struct gps_data_entry GPS_get_GPGGA_data(void);
struct gps_data_entry GPS_parse_data(char* fields);

void print_gps_to_LCD(struct gps_data_entry response);
void print_command_response_to_LCD(void);
void GPS_get_GPGGA_data_dump(void);
void GPS_erase_log(void);
void sendCommandToGPS(char* commandToSend);

int main(void) {

	init_GPS();
	GPS_erase_log();

	//Print out the GPS data values
	struct gps_data_entry response = GPS_get_GPGGA_data();
	print_gps_to_LCD(response);

	//Delay for some time
	int delay = 0;
	while(delay++ < delay_time){}

	//Get the data dump and try to print it out
	GPS_get_GPGGA_data_dump();
	print_command_response_to_LCD();

	return 0;
}

void print_command_response_to_LCD(void){
	//Print out gps data on LCD
			alt_up_character_lcd_dev * char_lcd_dev;
			// open the Character LCD port
			char_lcd_dev = alt_up_character_lcd_open_dev ("/dev/character_lcd_0");
			if ( char_lcd_dev == NULL){
				alt_printf ("Error: could not open character LCD device\n");
			}
			else{
				alt_printf ("\n Opened character LCD device\n");
				/* Initialize the character display */
				alt_up_character_lcd_init (char_lcd_dev);

				while(1){
					char value [2];
					value[0] = getcharGPS();
					value[1] = '\0';

					alt_up_character_lcd_string(char_lcd_dev, value);
				}
			}
}

void print_gps_to_LCD(struct gps_data_entry response){
	//Print out gps data on LCD
			alt_up_character_lcd_dev * char_lcd_dev;
			// open the Character LCD port
			char_lcd_dev = alt_up_character_lcd_open_dev ("/dev/character_lcd_0");
			if ( char_lcd_dev == NULL){
				alt_printf ("Error: could not open character LCD device\n");
			}
			else{
				alt_printf ("\n Opened character LCD device\n");
				/* Initialize the character display */
				alt_up_character_lcd_init (char_lcd_dev);

				/* Write Latitude and N_S indicator to the first row */
				alt_up_character_lcd_string(char_lcd_dev, response.latitude);
				alt_up_character_lcd_shift_cursor(char_lcd_dev, 1);
				alt_up_character_lcd_string(char_lcd_dev, response.n_s_indicator);

				/* Write Longitude and E_W indicator to the second row */
				alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
				alt_up_character_lcd_string(char_lcd_dev, response.longitude);
				alt_up_character_lcd_shift_cursor(char_lcd_dev, 1);
				alt_up_character_lcd_string(char_lcd_dev, response.e_w_indicator);
			}
}

struct gps_data_entry GPS_get_GPGGA_data(void){
	//"$GPGGA,231421.000,,,,,0,04,,,M,,M,,*7B

	int formats = 4;
	int format_count = 0;
	while(format_count++ < formats){

		//Wait until you get to the beginning of the command signaled by $
		while(getcharGPS() != '$');

		//Declare something to hold the fields you get back
		char fields[100] = {0};
		int i = 0;

		//Go through to the end of the command and save the values in fields
		char current = getcharGPS();
		while(current != '*'){
			fields[i++] = current;
			current = getcharGPS();
		}
		fields[i] = '\0';

		struct gps_data_entry gps_data = GPS_parse_data(fields);

		if(gps_data.used)
			return gps_data;
	}
}

struct gps_data_entry GPS_parse_data(char* fields){

	 struct gps_data_entry gps_entry;
	 gps_entry.used = 0;

	 if(fields[0] == 'G' && fields[1] == 'P' && fields[2] == 'G' && fields[3] == 'G' && fields[4] == 'A'){

		printf("\n field: %s", fields);
		printf("\n");

		//Assign the utc_time to the time to the gps entry
		int time_length = 6;
		int time_index = 0;

		while(*(fields + time_length) != ','){

			   gps_entry.utc_time[time_index] = *(fields + time_length);
			   time_length++;
			   time_index++;
		}
		gps_entry.utc_time[time_index] = '\0';


		//Assign the latitude time to the gps entry
		int latitude_length = 17;
		int latitude_index = 0;

		while(*(fields + latitude_length) != ','){
			   gps_entry.latitude[latitude_index] = *(fields + latitude_length);
			   latitude_length++;
			   latitude_index++;
		}
		gps_entry.latitude[latitude_index] = '\0';

		//Set the north / south indicator
		int n_s_length = 27;
		gps_entry.n_s_indicator[0] = *(fields + n_s_length);
		gps_entry.n_s_indicator[1] = '\0';

		//Assign the longitude time to the gps entry
		int longitude_length = 29;
		int longitude_index = 0;

		while(*(fields + longitude_length) != ','){
			   gps_entry.longitude[longitude_index] = *(fields + longitude_length);
			   longitude_length++;
			   longitude_index++;
		}
		gps_entry.longitude[longitude_index] = '\0';

		//Set the east / west indicator
		int e_w_length = 40;
		gps_entry.e_w_indicator[0] = *(fields + e_w_length);
		gps_entry.e_w_indicator[1] = '\0';

		if(DEBUG){

			printf("\n UTC Time ---- %s", gps_entry.utc_time);
			printf("\n Latitude ---- %s", gps_entry.latitude);
			printf("\n N/S Indicator ---- %s", gps_entry.n_s_indicator);
			printf("\n Longitude ---- %s", gps_entry.longitude);
			printf("\n E/W Indicator ---- %s", gps_entry.e_w_indicator);
		}

		gps_entry.used = 1;

		return gps_entry;

	 }

	 return gps_entry;
}

//Get the data dump
void GPS_get_GPGGA_data_dump(void){
	char* gps_command = "$PMTK622,1*29\r\n";
	sendCommandToGPS(gps_command);
}

//Erase the data log
void GPS_erase_log(void){
	char* gps_command = "$PMTK184,1*22\r\n";
	sendCommandToGPS(gps_command);
}

void sendCommandToGPS(char* commandToSend){
	int command_length = strlen(commandToSend);
	int submitted_commands = 0;
	int commandIndex = 0;

	int test_command;

	while(submitted_commands++ < command_length){
		test_command = putcharGPS(commandToSend[commandIndex++]);
	}
}

/*
 *  Subroutine to initialize the GPS Port by writing some data
 *  to the internal registers.
 *  Call this function at the start of the program before you attempt
 *  to read or write to data via the GPS port
 *  Refer to 6850 data sheet for details of registers
 *
 */
void init_GPS(void) {
	// set up 6850 Control Register to utilize a divide by 16 clock,
	// set RTS low, use 8 bits of data, no parity, 1 stop bit,
	// transmitter interrupt disabled
	GPS_Control = 0x15;

	//Set baud rate to 9600
	GPS_Baud		= 0x07;


}

char putcharGPS(char c) {
	// poll Tx bit in 6850 status register. Wait for it to become '1'
	while(GPSTestForTransmitData() == 0);

	// write 'c' to the 6850 TxData register to output the character
	GPS_TxData = c;

	return c; // return c
}

char getcharGPS(void) {
	// poll Rx bit in 6850 status register. Wait for it to become '1'
	while(GPSTestForReceivedData() == 0);

	// read received character from 6850 RxData register.

	int res = GPS_RxData;

	return res;
}

// the following function polls the 6850 to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read
int GPSTestForReceivedData(void) {
	// Test Rx bit in 6850 serial communications chip status register
	// if RX bit is set, return TRUE, otherwise return FALSE

	if ((GPS_Status & 0x1) == 0x1)
		return 1;
	else
		return 0;
}

int GPSTestForTransmitData(void) {
	// Test Tx bit in 6850 serial communications chip status register
	// if TX bit is set, return TRUE, otherwise return FALSE

	if ((GPS_Status & 0x2) == 0x2)
		return 1;
	else
		return 0;
}
