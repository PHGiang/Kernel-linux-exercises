// include functions: 
/*
1. convert a string array (numbers) -> integer array
2. sort 
3. integer array -> string array 
*/

#ifndef _STRING_SORT_H_
#define _STRING_SORT_H_

extern void rev_str(char *str, int len); 
extern int _atoi(char *str); 
extern void _itoa(int val, char* rs); 
extern void insertion_sort(int *arr, int len); 
extern void _in(int *rs, int *len_in, char *buf); 
extern void _out(char *rs, int *arr, int len); 

#endif