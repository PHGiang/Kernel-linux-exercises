#include "string_sort_lib.h"
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

void _in(int *rs, int *len_in, char *buf) {
	int i, j, k; 
	i = 0; j = 0; k = 0; 
	char tmp[11]; // max of an integer is 2 ... ... ... \null
	
	//int *rs = (int*)malloc(sizeof(int)*len); 

	while (1) {
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
		if (buf[i] == '\0')
			break; 
		i++; 
	}

	*len_in = k;	
}

void _out(char *rs, int *arr, int len)
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
}