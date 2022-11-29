CC = g++
CFLAGS = -Wall -g -O0

ifeq ($(OSTYPE),darwin)
  MQTTLIBDIR = -L/opt/homebrew/lib
else
  MQTTLIBDIR = -L/usr/lib/arm-linux-gnueabihf/
endif

all: can_send can_receive

can_send:can_send.c 
	    $(CC) $(CFLAGS) -o $@ $^ 

can_receive:can_receive.c 
	    $(CC) $(CFLAGS) -o $@ $^ 

i2ctest: i2ctest.c
	gcc -o $@  $^

container: container.cc
	$(CC) $(CFLAGS) -o $@  $^

mqtt: mqtt.cc
	$(CC) $(CFLAGS) $(MQTTLIBDIR) -lmosquittopp -o $@  $^


clean:
	    $(RM) can_send mqtt container .*.sw?
