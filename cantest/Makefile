CXX = c++
CXXFLAGS = -Wall -g -O0 -shared
LIB_LDFLAGS = 

ifeq ($(OSTYPE),darwin)
  MQTTLIBDIR = -L/opt/homebrew/lib -lmosquittopp
else
  MQTTLIBDIR = -L/usr/lib/arm-linux-gnueabihf/ -lmosquittopp 
endif

all: can_send can_receive

can_send:can_send.c 
	    $(CXX) $(CFLAGS) -o $@ $^ 

can_receive:can_receive.c 
	    $(CXX) $(CFLAGS) -o $@ $^ 

i2ctest: i2ctest.c
	gcc -o $@  $^

container: container.cc
	$(CXX) $(CFLAGS) -o $@  $^

mqtt: mqtt.o
	$(CXX) $(LIB_LDFLAGS) -o $@ $(MQTTLIBDIR) $^


clean:
	    $(RM) *.o can_send mqtt container .*.sw?
