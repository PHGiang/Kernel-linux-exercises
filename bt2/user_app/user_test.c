/*
* 2020/8/08
* app tren user space tuong tac voi char_device 
* char_device la mot thiet bi nam tren RAM
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include "string_sort_lib.h"

#define BUFFER_SIZE 512
#define DEVICE_NODE "/dev/char_dev"

#define MAGICAL_NUMBER 243
#define CLEAR_DATA_CHARDEV _IO(MAGICAL_NUMBER, 0)
#define GET_STATUS_CHARDEV  _IOR(MAGICAL_NUMBER, 1, status_t *)
#define CTRL_READ_CHARDEV  _IOW(MAGICAL_NUMBER, 2, unsigned char *)
#define CTRL_WRITE_CHARDEV _IOW(MAGICAL_NUMBER, 3, unsigned char *)

/* ham kiem tra entry point open cua char driver*/

typedef struct {
	unsigned char read_count_h; 
	unsigned char read_count_l; 
	unsigned char write_count_h; 
	unsigned char write_count_l; 
	unsigned char device_status; 
} status_t;



int open_chardev() {
	int fd = open(DEVICE_NODE, O_RDWR); 
	if (fd < 0) {
		printf ("Can not open the device file \n"); 
		exit(1); 
	}
	return fd; 
}

/* ham kiem tra entry point release cua char driver */
void close_chardev(int fd) {
	close(fd); 
}

/* ham kiem tra entry point read cua char driver */
void read_data_chardev() {
	int ret = 0; 
	char user_buf[BUFFER_SIZE]; 

	int fd = open_chardev(); 
	ret = read(fd, user_buf, BUFFER_SIZE); 
	close_chardev(fd); 

	if (ret < 0) 
		printf("Could not read a message from %s\n", DEVICE_NODE);
	else
		printf("Read a message from %s: %s\n", DEVICE_NODE, user_buf);
}

/* ham kiem tra entry point write cua char driver */
void write_data_chardev() {
	int ret = 0; 
	char user_buf[BUFFER_SIZE]; 
	printf("Enter your message: ");
	scanf(" %[^\n]s", user_buf);

	int fd = open_chardev(); 
	ret = write(fd, user_buf, strlen(user_buf) + 1); 
	close_chardev(fd); 

	if (ret < 0)
		printf("Could not write the messge to %s\n", DEVICE_NODE);
	else
		printf("Wrote the message to %s\n", DEVICE_NODE);
}

void sorted_write_chardev() {
	int ret = 0; 
	int arr[BUFFER_SIZE]; 
	int len_in; // number of input elements
	
	char user_buf[BUFFER_SIZE]; 

	printf("Enter your array: ");
	scanf(" %[^\n]s", user_buf); 

	_in(arr, &len_in, user_buf); 
	printf("Number of elements: %d\n", len_in);
	insertion_sort(arr, len_in); 
	_out(user_buf, arr, len_in);

	int fd = open_chardev(); 
	ret = write(fd, user_buf, strlen(user_buf) + 1); 
	close_chardev(fd); 

	if (ret < 0)
		printf("Could not write the sorted array to %s\n", DEVICE_NODE);
	else
		printf("Wrote the sorted array to %s\n", DEVICE_NODE);

}

void clear_data_chardev() {
	int fd = open_chardev(); 
	int ret = ioctl(fd, CLEAR_DATA_CHARDEV); 
	close_chardev(fd);
	printf("%s data register in char device \n", (ret < 0)?"Could'n clare":"Cleared");
}

void get_status_chardev(){
	status_t status; 
	unsigned int read_cnt, write_cnt; 
	int fd = open_chardev(); 
	ioctl(fd, GET_STATUS_CHARDEV, (status_t*)&status); 
	close_chardev(fd); 

	read_cnt = status.read_count_h << 8 | status.read_count_l; 
	write_cnt = status.write_count_h << 8 | status.write_count_l; 

	printf("Satistic: number of reading(%u), number of writing (%u)\n", read_cnt, write_cnt);

}

void control_read_chardev(){
	unsigned char isReadable = 0; 
	status_t status; 
	char c = 'n'; 
	printf("Do you want to enable reading from data registers (y/n)?"); 
	scanf(" %c", &c); 
	if (c == 'y') 
		isReadable = 1; 
	else if (c == 'n')
		isReadable = 0; 
	else 
		return; 

	int fd = open_chardev(); 
	ioctl(fd, CTRL_READ_CHARDEV, (unsigned char *)&isReadable); 
	ioctl(fd, GET_STATUS_CHARDEV, ((status_t*)&status)); 
	close_chardev(fd); 

	if (status.device_status & 0x01) 
		printf("Enable to read from data registers successful\n"); 
	else 
		printf("Disable to read from data registers successful\n"); 

}

void control_write_chardev(){
	unsigned char isWriteable = 0;
	status_t status; 
	char c = 'n'; 
	printf("Do you want to enable writing from data registers (y/n)?"); 
	scanf(" %c", &c); 
	if (c == 'y') 
		isWriteable = 1; 
	else if (c == 'n')
		isWriteable = 0; 
	else 
		return; 

	int fd = open_chardev(); 
	ioctl(fd, CTRL_WRITE_CHARDEV, (unsigned char *) &isWriteable); 
	ioctl(fd, GET_STATUS_CHARDEV, ((status_t*) &status)); 

	close_chardev(fd); 
	if (status.device_status & 0x02) 
		printf("Enable to write to data registers successful\n"); 
	else 
		printf("Disable to write to data registers successful\n"); 
}


int main() {
	//int ret = 0; 
	char option = 'q'; 
	int fd = -1; 

	printf("Select below options: \n"); 
	printf("\to (to open a device node)\n");
	printf("\tc (to close the device node)\n");
	printf("\tr (to read data from device node)\n"); 
	printf("\tw (to write data to device node)\n");
	printf("\tC (to clear data register)\n");
	printf("\tR (to enable/disable to read from data register)\n");
	printf("\tW (to enable/disable to write to data register)\n");
	printf("\tS (to sort then write an array to data register)\n");
	printf("\ts (to get status of device)\n"); 
	printf("\tq (to quit the application)\n"); 

	while (1) {
		
		printf("Enter you option: ");
		fflush(stdin);
		scanf(" %c", &option); 

		switch(option) {
			case 'o': 
				if (fd < 0) 
					fd = open_chardev(); 
				else 
					printf("%s has already opened.\n", DEVICE_NODE);
				break;
			case 'c': 
				if (fd > -1) 
					close_chardev(fd); 
				else 
					printf("%s has not opened yet! Can not close\n", DEVICE_NODE);
				break; 
			case 'r': 
				read_data_chardev(); 
				break;
			case 'w':
				write_data_chardev(); 
				break; 

			case 'C': 
			{
				clear_data_chardev(); 

			}
			break; 
			case 'R': 
			{
				control_read_chardev(); 
			}
			break;
			case 'W': 
			{
				control_write_chardev(); 
			}
			break; 
			case 'S': 
			{
				sorted_write_chardev(); 
			}
			break; 
			case 's': 
			{
				get_status_chardev(); 
			}
			break; 
			case 'q': 
				if (fd > -1)
					close_chardev(fd); 
				printf("Quit the application.\n");
				return 0; 
			default: 
				printf("invalid option %c.\n", option);
				break; 
		}
	}
	return 0; 
}
