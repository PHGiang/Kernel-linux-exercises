CC=gcc
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall
TARGET = user_test

all: user_test

user_test: user_test.o string_sort_lib.o	
	$(CC) $(CFLAGS) -o user_test user_test.o string_sort_lib.o

user_test.o: user_test.c string_sort_lib.h
	$(CC) $(CFLAGS) -c user_test.c 

string_sort_lib.o: string_sort_lib.c string_sort_lib.h
	$(CC) $(CFLAGS) -c string_sort_lib.c
clean: 
	$(RM) user_test *.o