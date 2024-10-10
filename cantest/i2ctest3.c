#include <stdio.h>
#include <stdlib.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define I2C_ADDR 0x41    // i2c address of i2c interface chip

int main(int argc, char *argv[]) {

  // -- create I2C bus
  int file;
  char *bus = "/dev/i2c-0";
  if ((file = open(bus, O_RDWR)) < 0) {
    printf("Failed to open the bus. \n");
    exit(1);
  }

  // -- get I2C device
  ioctl(file, I2C_SLAVE, I2C_ADDR);
  
  // -- send high repeatability measurement command
  //    command msb, command lsb(0x2C, 0x06)
  char config[2] = {0};
  config[0] = I2C_ADDR << 1; 
  write(file, config, 1);
  sleep(1);
  
  // -- read 6 bytes of data
  //    temp msb, temp lsb, temp CRC, humidity msb, humidity lsb, humidity CRC
  char data[6] = {0};
  read(file, data, 6);

  //   cout << "sbla ->" << sbla.str() << "<-"  << endl;
  char sbuffer[2];
  for (unsigned int i = 0; i < fdlen; ++i) {
    sprintf(sbuffer, " %02X", (int)data[i]);
    printf("sbuffer ");
  }
  
  return 0;
}
