CXX = c++
CXXFLAGS = -Wall -g -O0 -fPIC
LIB_LDFLAGS = -g -O0 -fPIC

ifeq ($(OSTYPE),darwin)
  MQTTLIBDIR = -L/opt/homebrew/lib -lmosquittopp
else
  MQTTLIBDIR = -L/usr/lib/arm-linux-gnueabihf/ -lmosquittopp 
endif

all: can_send can_receive rpi_i2c crc 

can_send:can_send.c 
	    $(CXX) $(CFLAGS) -o $@ $^ 

can_receive:can_receive.c 
	    $(CXX) $(CFLAGS) -o $@ $^ 

i2ctest: i2ctest.c
	gcc -o $@  $^

i2ctest2: i2ctest2.c
	gcc -o $@  $^

rpi_i2c: rpi_i2c.c
	gcc -o $@  $^ -lbcm_host

container: container.cc
	$(CXX) $(CFLAGS) -o $@  $^

prt: prt.cc
	$(CXX) $(CFLAGS) -o $@  $^

crc: crc.c
	$(CXX) $(CFLAGS) -o $@  $^

mqtt: mqtt.o
	$(CXX) $(LIB_LDFLAGS) -o $@ $^ $(MQTTLIBDIR) 


clean:
	    $(RM) *.o can_send mqtt container .*.sw?
