#include <stdio.h>
#include <stdlib.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define I2C_ADDR 0x44    // i2c address of sensor

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
  
  // -- send high repeatability measurement command
  //    command msb, command lsb(0x2C, 0x06)
  char config[2] = {0};
  /* config[0] = 0x2C;   // MSB */
  /* config[1] = 0x06;   // LSB */
  config[0] = 0x24;   // MSB
  config[1] = 0x00;   // LSB
  write(file, config, 2);
  sleep(1);
  
  // -- read 6 bytes of data
  //    temp msb, temp lsb, temp CRC, humidity msb, humidity lsb, humidity CRC
  char data[6] = {0};
  if (read(file, data, 6) != 6) {
    printf("Error : Input/output Error \n");
  } else {
    // -- convert the data
    //double cTemp = (((data[0] * 256) + data[1]) * 175.0) / 65535.0  - 45.0;
    //double humidity = (((data[3] * 256) + data[4])) * 100.0 / 65535.0;

    double cTemp = (((data[0]<<8) + data[1]) * 175.0) / 65535.0  - 45.0;
    double humidity = (((data[3]<<8) + data[4])) * 100.0 / 65535.0;
    
    // -- print
    printf("Temperature in Celsius : %.4f C \n", cTemp);
    printf("Relative Humidity is : %.4f RH \n", humidity);
  }
  
  return 0;
}
