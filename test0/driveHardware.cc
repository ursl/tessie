#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <net/if.h>
#include <sys/ioctl.h>


#include <chrono>
#include <thread>

#include "tLog.hh"

using namespace std;

// ----------------------------------------------------------------------
driveHardware::driveHardware(tLog& x, QObject *parent): QThread(parent), fLOG(x) {
  fRestart   = false;
  fAbort     = false;
  fCANReg    = 0;
  fCANVal    = 0.;
  QDateTime dt = QDateTime::currentDateTime();
  fDateAndTime = dt.date().toString() + "  " +  dt.time().toString("hh:mm");

  fRpcThread = new QThread();
  fRpcServer = new rpcServer(this);
  //  connect(fRpcThread, &QThread::finished, fRpcServer, &QObject::deleteLater);
  connect(this, &driveHardware::sendToServer, fRpcServer, &rpcServer::sentToServer);
  connect(this, &driveHardware::startServer, fRpcServer, &rpcServer::run);
  connect(fRpcServer, &rpcServer::sendFromServer, this, &driveHardware::sentFromServer);
  fRpcServer->moveToThread(fRpcThread);
  fRpcThread->start();
  emit startServer();
  //  fRpcServer->run();

#ifdef PI
  // -- write CAN socket
  memset(&fFrameW, 0, sizeof(struct can_frame));

  fSw = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fSw < 0) {
    perror("socket PF_CAN failed");
    return;
  }
    
  strcpy(fIfrW.ifr_name, "can0");
  int ret = ioctl(fSw, SIOCGIFINDEX, &fIfrW);
  if (ret < 0) {
    perror("ioctl failed");
    return;
  }
  
  fAddrW.can_family = AF_CAN;
  fAddrW.can_ifindex = fIfrW.ifr_ifindex;
  ret = bind(fSw, (struct sockaddr *)&fAddrW, sizeof(fAddrW));
  if (ret < 0) {
    perror("bind failed");
    return;
  }

  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  
  // -- read CAN socket
  memset(&fFrameR, 0, sizeof(struct can_frame));

  fSr = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fSr < 0) {
    perror("socket PF_CAN failed");
    return;
  }
    
  strcpy(fIfrR.ifr_name, "can0");
  ret = ioctl(fSr, SIOCGIFINDEX, &fIfrR);
  if (ret < 0) {
    perror("ioctl failed");
    return;
  }
  
  fAddrR.can_family = AF_CAN;
  fAddrR.can_ifindex = fIfrR.ifr_ifindex;
  ret = bind(fSr, (struct sockaddr *)&fAddrR, sizeof(fAddrR));
  if (ret < 0) {
    perror("bind failed");
    return;
  }
    
  //4.Define receive rules
  struct can_filter rfilter[1];
  rfilter[0].can_id = 0x000;
  rfilter[0].can_mask = CAN_SFF_MASK;
  setsockopt(fSr, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

  
#endif
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {

  fMutex.lock();
  fAbort = true;
  fCondition.wakeOne();
  fMutex.unlock();

  fRpcThread->quit();
  fRpcThread->wait();

  wait();
#ifdef PI
  shutDown();
#endif
}

// ----------------------------------------------------------------------
void driveHardware::sentFromServer(const QString &result) {
  cout << "driveHardware::sentFromServer() -> " << result.toStdString() << "<-" << endl;
  signalText(result);
}


// ----------------------------------------------------------------------
void  driveHardware::getMessage(std::string x) {
  QString aline(QString::fromStdString(x));
  signalText(aline);

  // -- do something else eventually
}


// ----------------------------------------------------------------------
void  driveHardware::printToGUI(std::string x) {
  QString aline(QString::fromStdString(x));
  signalText(aline);
}


// ----------------------------------------------------------------------
void driveHardware::runPrintout(int reg, float val) {
  cout << "driveHardware::runPrintout() " << endl;
  QMutexLocker locker(&fMutex);
  this->fCANReg = reg;
  this->fCANVal = val;

  if (!isRunning()) {
    start(LowPriority);
  } else {
    fRestart = true;
    fCondition.wakeOne();
  }

}


// ----------------------------------------------------------------------
void driveHardware::run() {
  std::chrono::milliseconds oneSec(1000);
  cout << "Hallo in run()" << endl;
  int cnt(0);
  while (1) {
    ++cnt;
    if (cnt%10 == 1) {
      cout << "Hallo in run(), cnt = " << cnt << endl;
    }
    //    readCANmessage();
    entertainFras();
    std::this_thread::sleep_for(oneSec);

  }

}


// ----------------------------------------------------------------------
void driveHardware::setRegister(int reg) {
  fCANReg = reg;
}

// ----------------------------------------------------------------------
void driveHardware::setValue(float val) {
  fCANVal = val;
}

// ----------------------------------------------------------------------
void driveHardware::setId(int val) {
  fCANId = val;
}


// ----------------------------------------------------------------------
int driveHardware::getRegister() {
  return fCANReg;
}

// ----------------------------------------------------------------------
float driveHardware::getValue() {
  return fCANVal;
}

// ----------------------------------------------------------------------
int driveHardware::getId() {
  return fCANId;
}


#ifdef PI
// ----------------------------------------------------------------------
void driveHardware::shutDown() {
  // FIXME: do a poff for all TEC controllers. set all relais outputs to 0.
  close(fSw);
}


// ----------------------------------------------------------------------
void driveHardware::sendCANmessage() {
  //  fMutex.lock();
  char data[4] = {0, 0, 0, 0};
  fFrameW.can_id = fCANId;
  int dlength(0), command(0);
  if (0x0 == ((0x0f0 & fCANId)>>4)) {
    // -- x0x is command access and no subsequent 4 bytes are required. fCANReg indicates the command type
    dlength = 1;
    command = 1; 
  }
  if (0x1 & ((0x0f0 & fCANId)>>4)) {
    // -- x1x is read access and no subsequent 4 bytes are required
    dlength = 1;
  }
  if (0x2 & ((0x0f0 & fCANId)>>4)) {
    // -- x2x is write access and the subsequent 4 bytes are the value to be written
    dlength = 5;
  }
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fCANReg;
  if (dlength > 1) {
    memcpy(data, &fCANVal, sizeof fCANVal);
    fFrameW.data[1] = data[0];
    fFrameW.data[2] = data[1];
    fFrameW.data[3] = data[2];
    fFrameW.data[4] = data[3];
  }
  if (1 == command) {
    cout << "sendCANmessage: canid = " << fCANId << " cmd = " << fCANReg
         << endl;
  } else {
    cout << "canid = " << fCANId << " reg = " << fCANReg
         << " value = " << fCANVal
         << " dlength = " << dlength
         << endl;

  }
  printf("can_id  = 0x%X (from sendCANmessage())\n", fFrameW.can_id);
  printf("can_dlc = %d\n", fFrameW.can_dlc);

  for (int i = 0; i < fFrameW.can_dlc; ++i) {
    printf("data[%d] = %2x/%3d\r\n", i, fFrameW.data[i], fFrameW.data[i]);
  }
    
  //6.Send message
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW)); 
  if (nbytes != sizeof(fFrameW)) {
    printf("Send Error frame[0]!\r\n");
  }


  //  fMutex.unlock();
}


// ----------------------------------------------------------------------
void driveHardware::entertainFras() {
  if (0 == fValveMask) {
    fFrameW.can_id = CAN_RTR_FLAG | 0x41;
    int dlength(0);
    fFrameW.can_dlc = dlength;
    // stringstream sbla; sbla << "entertainFras "
    //                         << " reg = 0x"  << hex << fFrameW.can_id;
    // cout << "sbla: " << sbla.str() << endl;
    
    // -- Send message
    setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    int nbytes = write(fSw, &fFrameW, sizeof(fFrameW)); 
    if (nbytes != sizeof(fFrameW)) {
      printf("Send Error frame[0]!\r\n");
    }
  } else {
    fFrameW.can_id = 0x40;
    int dlength(1);
    fFrameW.can_dlc = dlength;
    fFrameW.data[0] = fValveMask;
    
    // -- Send message
    setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    int nbytes = write(fSw, &fFrameW, sizeof(fFrameW)); 
    if (nbytes != sizeof(fFrameW)) {
      printf("Send Error frame[0]!\r\n");
    }
  }
}


// ----------------------------------------------------------------------
void driveHardware::talkToFras() {
  fValveMask = static_cast<int>(fCANVal);
  cout << "talkToFras"  << endl;
  //  fMutex.lock();

  //            TEC:   ssP'..tt'aaaa        
  //           FRAS:   0aa'aaaa'akkk
  //  fFrameW.can_id = 000'0100'0000 -> 0x040 for process
  //  fFrameW.can_id = 000'0100'0001 -> 0x041 for service
  //  fFrameW.can_id = 000'0100'0010 -> 0x042 for control

  fFrameW.can_id = 0x40;
  int dlength(1);
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fValveMask;
  stringstream sbla; sbla << "talkToFras "
                          << " reg = 0x"  << hex << fFrameW.can_id
                          << " data = " << fCANVal;
  cout << "sbla: " << sbla.str() << endl;
  fLOG(INFO, sbla.str());

  // -- Send message
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW)); 
  if (nbytes != sizeof(fFrameW)) {
    printf("Send Error frame[0]!\r\n");
  }

  //  fMutex.unlock();

}


// ----------------------------------------------------------------------
void driveHardware::readCANmessage() { 
  static int cntCAN(0);
  
  int nbytes(0); 
  char data[4]; 
  unsigned int idata(0);
  float fdata(0.0);
  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
  cout << "readCANmessage(), nbytes = " << nbytes << endl;
  if(nbytes > 0) {
    printf("can_id = 0x%X ncan_dlc = %d (from run())\n", fFrameR.can_id, fFrameR.can_dlc);
    int i = 0;
      
    for(i = 0; i < fFrameR.can_dlc; i++) {
      printf("data[%d] = %2x/%3d\n", i, fFrameR.data[i], fFrameR.data[i]);
    }

    int reg = fFrameR.data[0];
    data[0] = fFrameR.data[1];
    data[1] = fFrameR.data[2];
    data[2] = fFrameR.data[3];
    data[3] = fFrameR.data[4];
      
    memcpy(&fdata, data, sizeof fdata); 
    memcpy(&idata, data, sizeof idata); 
    printf("float = %f/uint32 = %u\n", fdata, idata);
    ++cntCAN;
    printf("received CAN message %d\n", cntCAN);
    stringstream sbla; sbla << "CAN read "
                            << " reg = 0x"  << hex << reg
                            << " value = " << fdata;
    cout << "sbla: " << sbla.str() << endl;
    fLOG(INFO, sbla.str());

  }
  
  return;
}

#endif
