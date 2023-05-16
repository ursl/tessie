#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define I2C_ADDR1 0x3e    // i2c address of 
#define I2C_ADDR2 0x3f    // i2c address of 

// ----------------------------------------------------------------------
void readI2c(int file) {
  // -- read 16 bytes of data
  char data[16] = {0};
  int readl = read(file, data, 16); 
  if (readl != 16) {
    printf("Error : Input/output Error readl = %d\n", readl);
  } else {
    printf("Bytes read: ");
    for (int i = 0; i < 16; i = i+2) {
      printf("%02x%02x ", data[i], data[i+1]);
    }
    printf("\n");          
  }
}


// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  // -- create I2C bus
  int file;
  char *bus = "/dev/i2c-0";
  if ((file = open(bus, O_RDWR)) < 0) {
    printf("Failed to open the bus. \n");
    exit(1);
  }

  ioctl(file, I2C_SLAVE, I2C_ADDR1);
  readI2c(file); 

  ioctl(file, I2C_SLAVE, I2C_ADDR2);
  readI2c(file); 

  return 0;
}
