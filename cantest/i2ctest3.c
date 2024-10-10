#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
  
  // -- set command byte to 0x0 (Register: Input Port, Protocol: Read Byte)
  char config = 0x0;
  if (1) {
    write(file, &config, 1);
  }
  /* write(file, config+2, 1); */
  /* write(file, config+1, 1); */

  // -- read 1 bytes of data
  int dlen = 1;
  char data = 0;
  read(file, &data, 1);

  printf("read back: %x\n", (char)~data);
  return 0;
}
