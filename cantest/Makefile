CC = g++
CFLAGS = -Wall -g -O0

all: can_send can_receive

can_send:can_send.c 
	    $(CC) $(CFLAGS) -o $@ $^ 

can_receive:can_receive.c 
	    $(CC) $(CFLAGS) -o $@ $^ 

i2ctest: i2ctest.c
	gcc -o $@  $^

container: container.cc
	$(CC) $(CLFAGS) -o $@  $^


clean:
	    $(RM) can_send .*.sw?
