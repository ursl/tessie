#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/*
First do: 
========
sudo ip link set can0 type can bitrate 125000 
sudo ip link set can0 up

NOTE: Initially 125kbit/s should be 1Mbit/s
*/

int main() {
  int ret;
  int s, nbytes;
  int cnt;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct can_frame frame;

  char data[4] = {0, 0, 0, 0};
  unsigned int idata;
  float fdata; 
  
  memset(&frame, 0, sizeof(struct can_frame));
    
  /* 
     system("sudo ip link set can0 type can bitrate 100000");
     system("sudo ifconfig can0 up");
  */
    
  //1.Create socket
  s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s < 0) {
    perror("socket PF_CAN failed");
    return 1;
  }
    
  //2.Specify can0 device
  strcpy(ifr.ifr_name, "can0");
  ret = ioctl(s, SIOCGIFINDEX, &ifr);
  if (ret < 0) {
    perror("ioctl failed");
    return 1;
  }

  //3.Bind the socket to can0
  addr.can_family = PF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("bind failed");
    return 1;
  }
    
  //4.Define receive rules
  struct can_filter rfilter[1];
  rfilter[0].can_id = 0x000;
  rfilter[0].can_mask = CAN_SFF_MASK;
  setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

  //5.Receive data and exit
  cnt = 0; 
  while(1) {
    while(1) {
      nbytes = read(s, &frame, sizeof(frame));
      if(nbytes > 0) {
        printf("can_id = 0x%X\r\ncan_dlc = %d \r\n", frame.can_id, frame.can_dlc);
        int i = 0;

        for(i = 0; i < frame.can_dlc; i++) {
          printf("data[%d] = %2x/%3d\r\n", i, frame.data[i], frame.data[i]);
        }

        data[0] = frame.data[1];
        data[1] = frame.data[2];
        data[2] = frame.data[3];
        data[3] = frame.data[4];

        memcpy(&fdata, data, sizeof fdata); 
        memcpy(&idata, data, sizeof idata); 
        printf("float = %f/uint32 = %u\r\n", fdata, idata);
        ++cnt;
        break;
      }
    }
      
    printf("received message %d\r\n", cnt);
  }
  //6.Close the socket and can0
  close(s);
  system("sudo ifconfig can0 down");
    
  return 0;
}
