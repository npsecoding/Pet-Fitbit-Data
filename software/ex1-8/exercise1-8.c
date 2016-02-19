#include <stdio.h>
#include <altera_up_sd_card_avalon_interface.h>

void writeTest();
void readTest();
char buffer[512] = "Writing to the SD card is fun!\r\n\0";
char data[512];

short int myFileHandle;
short int myFileHandle2;

int main(void) {
	printf("SD Card Access Test\n");

	alt_up_sd_card_dev *device_reference = NULL;
	int connected = 0;

	printf("Opening SDCard\n");
	device_reference = alt_up_sd_card_open_dev("/dev/Altera_UP_SD_Card_Avalon_Interface_0");
	if (device_reference == NULL) {
		printf("SDCard Open FAILED\n");
		return 0;
	} else{
		printf("SDCard Open PASSED\n");
		while (1) {
			if ((connected == 0) && (alt_up_sd_card_is_Present())) {
				printf("Card connected.\n");
				if (alt_up_sd_card_is_FAT16()) {
					printf("FAT16 file system detected.\n\n");
					writeTest();
					readTest();
				} else {
					printf("Unknown file system.\n");
				}
				connected = 1;
			} else if ((connected == 1) && (alt_up_sd_card_is_Present() == false)) {
				printf("Card disconnected.\n");
				connected = 0;
			}
		}
	}

	return 0;
}

void writeTest(){
	printf("\n\nWriting Test\n");

	bool write_signal;
	myFileHandle = alt_up_sd_card_fopen("test1.txt", false);			// create a new file test.txt to write to
	if(myFileHandle != -1) {
		printf("File was created and opened\n");
		int index = 0;
		// write data in the buffer to the file
		while(buffer[index] != '\0'){
			write_signal = alt_up_sd_card_write(myFileHandle, buffer[index]);
			if(write_signal == false) {
				printf("Error writing to file...\n");
				return;
			}
			index++;
		}
		printf("Writing Done!!!\n");
		printf("Closing file...\n");
		alt_up_sd_card_fclose(myFileHandle);
	}
	else
		printf("File NOT created or opened\n");
}


void readTest(){
	printf("\n\nReading Test\n");
	myFileHandle2 = alt_up_sd_card_fopen("test1.txt", false);			// create a new file test.txt to write to
	if(myFileHandle2 < 0)
		printf("Problem creating file. Error %i\n", myFileHandle2);
	else{
		printf("SD accesses successfully, reading data...\n");
		printf("Data read from file = \n");
		int i;
		for (i = 0; i < 32; i++) {	//  data32 is the new line character \n
			data[i] = alt_up_sd_card_read(myFileHandle2);
			printf("%c", data[i]);
		}
		printf("\nClosing file after reading...\n");
		alt_up_sd_card_fclose(myFileHandle2);
		printf("We are done reading!\n");
	}
}
