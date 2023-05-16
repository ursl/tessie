#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define I2C_ADDR 0x3f    // i2c address of 

int main(int argc, char *argv[]) {

  // -- create I2C bus
  int file;
  char *bus = "/dev/i2c-0";
  if ((file = open(bus, O_RDWR)) < 0) {
    printf("Failed to open the bus. \n");
    exit(1);
  }

  // -- get I2C device, SHT85 I2C address is 0x44
  ioctl(file, I2C_SLAVE, I2C_ADDR);
  
  // -- read 16 bytes of data
  char data[16] = {0};
  int readl = read(file, data, 16); 
  if (readl != 16) {
    printf("Error : Input/output Error readl = %d\n", readl);
  } else {
    printf("Bytes read: ");
    for (int i = 0; i < 16; ++i) {
      printf("%2x ", data[i]);
    }
    printf("\n");          
  }
  
  return 0;
}
