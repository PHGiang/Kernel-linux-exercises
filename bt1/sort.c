#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// int len_in; // number of elements of input 
// int len_out; 

void rev_str(char *str, int len) {

	int n = len >> 1;
	int i = 0;
	char tmp;   
	for (i = 0; i < n; i++) {
		tmp = str[len - 1 - i];
		str[len - 1 - i] = str[i];
		str[i] = tmp;  
	}
}

int _atoi(char *str)
{
	int i, rs; 
	rs = 0;
	i = 0;  
	while (str[i] != '\0') {
		rs = (rs << 3) + (rs << 1) + (str[i] - '0');
		i++;  
	}
	return rs; 	
}

void _itoa(int val, char* rs) {
	//char rs[11]; // max of an integer is 2 ... ... ... \null
	int i = 0;
	while (val != 0) {
		rs[i++] = val%10 + '0';
		val = val/10; 
	}
	rs[i] = '\0';
	rev_str(rs, i); 
}

void insertion_sort(int *arr, int len)
{
	int i, j, c; 
	for (i = 1; i < len; i++) {
		c = arr[i]; 
		j = i - 1; 
		while (j >= 0 && arr[j] > c) {
			arr[j+1] = arr[j]; 
			j--; 	
		}
		arr[++j] = c; 
	}
}

void print_arr(int *arr, int len) {
	int i; 
	for (i = 0; i < len; i++) {
		printf("%d\t", arr[i]); 
	}
	printf("\n"); 
}

int* _in(char *buf, int len, int *len_in) {
	int i, j, k; 
	i = 0; j = 0; k = 0; 
	char tmp[11]; // max of an integer is 2 ... ... ... \null
	
	int *rs = (int*)malloc(sizeof(int)*len); 

	while (i <= len) {
		if (buf[i] >= '0' && buf[i] <= '9') { 
			tmp[j++] = buf[i]; 
		}
		else 
		{
			tmp[j] = '\0'; 
			rs[k] = _atoi(tmp); 
			//printf("%d ", rs[k]); 
			k++;  
			j = 0; 
		}
		i++; 
	}

	 
	*len_in = k;
	return rs;  
}

void _out(int *arr, int len, char *rs, int *len_out)
{
	int i, j, k;
	j = 0; 
	char tmp[11]; 

	for (i = 0; i < len; i++) {
		_itoa(arr[i], tmp);
		
		k = 0;  
		while(tmp[k] != '\0')
		{
			rs[j++] = tmp[k++]; 
		}	
		rs[j++] = ' '; 
	}
	j--; 
	rs[j] = '\0';
	*len_out = j;  
	
}

int main(int argc, char*argv[]) {
	int i; // length of input
	/*
	int len = argc - 1; 
	int *arr = (int*)malloc(sizeof(int)*len);  
	
	for (i = 0; i < len; i++) {
		arr[i] = _atoi(argv[i+1]); 
	}
	*/
	int len_in, len_out; 
	 
	char buf[128]; 
	int nread; 
	// in = open("input_arr.txt", O_RDONLY); 
	
	/*READ DATA TO BUFFER*/
	//input_arr.txt: opened file, fd: 0
	nread = read(0, buf, 128); 
	if (nread == -1) 
		write(2, "A read error has occured\n", 26);

	 /* PROCESS INPUT DATA (char *) TO AN INTEGER ARRAY */
	int *arr = _in(buf, nread, &len_in);
	
	/* SORT */
	insertion_sort(arr, len_in);
	
	int out = open("output_arr.txt", O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR);
	
	/* PROCESS SORTED ARRAY A CHAR ARRAY*/	
	_out(arr, len_in, buf, &len_out);
	
	/*WRITE THE SORTED ARRAY TO OUTPUT FILE*/
	if (write(out, buf, len_out) != nread)
		write(2, "A write error has occured\n", 27);
	 
	return 0; 
}
