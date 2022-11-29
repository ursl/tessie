CC = c++
CFLAGS = -Wall -g -O0 -shared
LIB_LDFLAGS = 

ifeq ($(OSTYPE),darwin)
  MQTTLIBDIR = -L/opt/homebrew/lib -lmosquittopp
else
  MQTTLIBDIR = -L/usr/lib/arm-linux-gnueabihf/ -lmosquittopp 
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

mqtt: mqtt.o
	$(CC) $(LIB_LDFLAGS) $(MQTTLIBDIR) -o $@  $^


clean:
	    $(RM) *.o can_send mqtt container .*.sw?
