#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// gcc -Wall -o spi spi.c -lwiringPi
// https://indico.cern.ch/event/661919/sessions/260190/attachments/1686227/2711616/Slides_HV.pdf

int main (void) {
  printf("hallo start\n");

  int spi_fd1;
  unsigned char spi_buf[3];
  
  // setup IO
  wiringPiSetup();
  pinMode(8, OUTPUT); // pin #3

  //  spi_fd0 = wiringPiSPISetup(0, 10000000);// DAC 10 MHz
  spi_fd1 = wiringPiSPISetup(1, 10000000);// ADC 10 MHz

  //??  wiringPiSPISetup(1, 1000000);
  
  // program DAC range
  spi_buf[0] = 0x08;
  spi_buf[1] = 0x00;
  spi_buf[2] = 0x00;
  // Range = 0...+10V
  wiringPiSPIDataRW(spi_fd1, spi_buf, 3);

  printf("spi_buf = %x %x %x\n", spi_buf[0], spi_buf[1], spi_buf[2]);
  

  printf("hallo end\n");

  return 0 ;
}
