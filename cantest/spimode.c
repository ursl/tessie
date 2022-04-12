#include <stdio.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
   int fd;
   char mode;

   fd = open("/dev/spidev0.1", O_RDWR);

   if (fd >= 0) {
      /* write mode */
      mode = SPI_MODE_3;
      ioctl(fd,SPI_IOC_WR_MODE,&mode);

      /* read mode */
      ioctl(fd,SPI_IOC_RD_MODE,&mode);
      printf("mode = %u\n",mode);

      /* write mode */
      mode = SPI_MODE_1;
      ioctl(fd,SPI_IOC_WR_MODE,&mode);

      /* read mode */
      ioctl(fd,SPI_IOC_RD_MODE,&mode);
      printf("mode = %u\n",mode);
   } else {
     printf("could not open /dev/spidev0.1\n");
   }

   close(fd);

   return 0;
}
