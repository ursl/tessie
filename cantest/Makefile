CC = g++
CFLAGS = -Wall -g -O0

all: can_send can_receive

can_send:can_send.c 
	    $(CC) $(CFLAGS) -o $@ $^ 

can_receive:can_receive.c 
	    $(CC) $(CFLAGS) -o $@ $^ 

clean:
	    $(RM) can_send .*.sw?
