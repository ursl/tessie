#include <stdio.h>
#include <stdlib.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/*#include <wiringPiI2C.h>*/

#define I2C_ADDR 0x44    // i2c address of sensor

int main(int argc, char *argv[]) {


  // Create I2C bus
  int file;
  char *bus = "/dev/i2c-0";
  if((file = open(bus, O_RDWR)) < 0) {
      printf("Failed to open the bus. \n");
      exit(1);
	}
  // Get I2C device, SHT31 I2C address is 0x44(68)
  ioctl(file, I2C_SLAVE, I2C_ADDR);

// Send high repeatability measurement command
	// Command msb, command lsb(0x2C, 0x06)
	char config[2] = {0};
	config[0] = 0x2C;
	config[1] = 0x06;
	write(file, config, 2);
	sleep(1);
 
	// Read 6 bytes of data
	// temp msb, temp lsb, temp CRC, humidity msb, humidity lsb, humidity CRC
	char data[6] = {0};
	if (read(file, data, 6) != 6) {
      printf("Error : Input/output Error \n");
	} else {
      // Convert the data
      double cTemp = (((data[0] * 256) + data[1]) * 175.0) / 65535.0  - 45.0;
      double fTemp = (((data[0] * 256) + data[1]) * 315.0) / 65535.0 - 49.0;
      double humidity = (((data[3] * 256) + data[4])) * 100.0 / 65535.0;
      
      // Output data to screen
      printf("Temperature in Celsius : %.2f C \n", cTemp);
      printf("Temperature in Fahrenheit : %.2f F \n", fTemp);
      printf("Relative Humidity is : %.2f RH \n", humidity);
	}

  
  /*
  // Setup I2C communication
  int fd = wiringPiI2CSetup(I2C_ADDR);
  if (fd == -1) {
    printf("Failed to init I2C communication.\n");
    // return -1;
  }
  printf("I2C communication successfully setup.\n");
  printf("Init result: %d\n", fd);
  */

  
  // Switch device to measurement mode
  //wiringPiI2CWriteReg8(fd, I2C_ADDR, 0b00001000);
  
  //  int result = wiringPiI2CWriteReg16(fd, I2C_ADDR, 0x0024);
  /*
  unsigned char test[6] = {0};
  // data to be sent
  test[0]= 0x00;
  test[1]= 0x24;

  wiringPiI2CWrite(fd, test
  */
  
  /*
  printf("write to i2c\n");
  write(fd, test, 2);
  
  sleep(1);
  read(fd, test, 6);
  int i;
  for (i = 0; i < 6; ++i) {
    printf("%02X ", test[i]);
  }
  printf("\n");
  */
  
  /* int result = wiringPiI2CWriteReg8(fd, I2C_ADDR, 0x24); */
  /* if (result == -1) printf("Error: %d\n", result); */

  /* result = wiringPiI2CWriteReg8(fd, I2C_ADDR, 0x00); */
  /* if (result == -1) printf("Error: %d\n", result); */

  return 0;
}
