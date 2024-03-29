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

int main(int argc, char *argv[]) {
  int i;

  int id, reg; 
  char data[4] = {0, 0, 0, 0};
  float fdata; 
  unsigned int idata;
  int dlength(1);
  
  int ret;
  int s, nbytes;
  int first;
  
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct can_frame frame;


  memcpy(&fdata, data, sizeof fdata); 
  memcpy(&idata, data, sizeof idata); 

  // -- command line arguments
  for (i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-i"))  {id    = strtol(argv[++i], NULL, 16); }     // can bus ID
    if (!strcmp(argv[i],"-r"))  {reg   = strtol(argv[++i], NULL, 16); }     // register address

    if (!strcmp(argv[i],"-d"))  {
      printf("parse -d\n");
      dlength += 4;
      data[0]  = static_cast<char>(strtoul(argv[i+1], NULL, 16));
      data[1]  = static_cast<char>(strtoul(argv[i+2], NULL, 16));
      data[2]  = static_cast<char>(strtoul(argv[i+3], NULL, 16));
      data[3]  = static_cast<char>(strtoul(argv[i+4], NULL, 16));

      memcpy(&fdata, data, sizeof fdata); 
      memcpy(&idata, data, sizeof idata); 

    }     

    if (!strcmp(argv[i],"-f"))  {
      printf("parse -f\n");
      dlength += 4;
      fdata = atof(argv[++i]);
      memcpy(data, &fdata, sizeof fdata);
      memcpy(&idata, data, sizeof idata); 
    }     

    if (!strcmp(argv[i],"-u"))  {
      printf("parse -u\r\n");
      dlength += 4;
      idata = atoi(argv[++i]);
      fdata = 0.0; 
      memcpy(data, &idata, sizeof idata); 
      memcpy(&fdata, data, sizeof fdata); 
    }     
  }

  
  printf("id = %d reg = %d, data[0..3] = %d %d %d %d/%x %x %x %x (f: %f, u: %u)\n",
         id, reg, data[0], data[1], data[2], data[3],
         data[0], data[1], data[2], data[3],
         fdata, idata);
  
  memset(&frame, 0, sizeof(struct can_frame));

  if (0) {
    printf("setup link \r\n");
    system("sudo ip link set can0 type can bitrate 125000");
    printf("ifconfig can0 \r\n");
    system("sudo ifconfig can0 up");
  }
        
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
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("bind failed");
    return 1;
  }
    
  //4.Disable filtering rules, do not receive packets, only send
  setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  //5.Set send data
  frame.can_id = id;
  frame.can_dlc = dlength;
  frame.data[0] = reg;
  if (dlength > 1) {
    frame.data[1] = data[0];
    frame.data[2] = data[1];
    frame.data[3] = data[2];
    frame.data[4] = data[3];
  }
  printf("can_id  = 0x%X\r\n", frame.can_id);
  printf("can_dlc = %d\r\n", frame.can_dlc);

  for(i = 0; i < frame.can_dlc; ++i) {
    printf("data[%d] = %2x/%3d\r\n", i, frame.data[i], frame.data[i]);
  }
    
  //6.Send message
  nbytes = write(s, &frame, sizeof(frame)); 
  if(nbytes != sizeof(frame)) {
    printf("Send Error frame[0]!\r\n");
    /*    system("sudo ifconfig can0 down");*/
  }
    
  //7.Close the socket and can0
  close(s);
  /*  system("sudo ifconfig can0 down");*/
  return 0;
}
