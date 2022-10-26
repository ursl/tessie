#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <net/if.h>
#include <sys/ioctl.h>

#ifdef PI
#include <errno.h>
#include <wiringPiI2C.h>
#endif

#include <chrono>
#include <thread>

#include "tLog.hh"

using namespace std;

// ----------------------------------------------------------------------
driveHardware::driveHardware(tLog& x, QObject *parent): QThread(parent), fLOG(x) {
  fParent    = parent;
  fRestart   = false;
  fAbort     = false;
  fCANReg    = 0;
  fCANVal    = 0.;
  QDateTime dt = QDateTime::currentDateTime();
  fDateAndTime = dt.date().toString() + "  " +  dt.time().toString("hh:mm");

  fCANReadIntVal = 3;
  fCANReadFloatVal = 3.1415;

  fMilli5   = std::chrono::milliseconds(5);
  fMilli10  = std::chrono::milliseconds(10);
  fMilli100 = std::chrono::milliseconds(100);

  fCsvFileName = "tessie.csv";
  cout << "open " << fCsvFileName << endl;
  fCsvFile.open(fCsvFileName, ios_base::app);
  cout << "done?" << endl;\

  for (unsigned int itec = 1; itec <= 4; ++itec) {
    fActiveTEC.insert(make_pair(itec, 0));
  }
  for (unsigned int itec = 5; itec <= 8; ++itec) {
    fActiveTEC.insert(make_pair(itec, 1));
  }
  fNActiveTEC = 4;

  initTECData();

#ifdef PI
  if (0) {
  int fd = wiringPiI2CSetup(0x44);

  cout << "Init result: "<< fd << endl;
  int result = wiringPiI2CWriteReg16(fd, 0x44, 0x2400 );
  if (result == -1) {
     cout << "Error.  Errno is: " << errno << endl;
  }
  sleep(1);
  unsigned int data[6];
  data[0] = wiringPiI2CReadReg8(fd, 0x44);
  cout << "data[0] = " << data[0] << endl;
  data[1] = wiringPiI2CReadReg8(fd, 0x44);
  cout << "data[1] = " << data[1] << endl;
  data[2] = wiringPiI2CReadReg8(fd, 0x44);
  cout << "data[2] = " << data[2] << endl;
  data[3] = wiringPiI2CReadReg8(fd, 0x44);
  cout << "data[3] = " << data[3] << endl;
  data[4] = wiringPiI2CReadReg8(fd, 0x44);
  cout << "data[4] = " << data[4] << endl;
  data[5] = wiringPiI2CReadReg8(fd, 0x44);
  cout << "data[5] = " << data[5] << endl;
  }
#endif

  //rpc  fRpcThread = new QThread();
  //rpc  fRpcServer = new rpcServer(this);
  //rpc  connect(this, &driveHardware::sendToServer, fRpcServer, &rpcServer::sentToServer);
  //rpc  connect(this, &driveHardware::startServer, fRpcServer, &rpcServer::run);
  //rpc  connect(fRpcServer, &rpcServer::sendFromServer, this, &driveHardware::sentFromServer);
  //rpc  fRpcServer->moveToThread(fRpcThread);
  //rpc  fRpcThread->start();
  //rpc  emit startServer();

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
  // -- this does not work with the new CANBUS protocol?!
  // setsockopt(fSr, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));


  // -- add timeout?
  // https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
  // struct timeval tv;
  // tv.tv_sec = 0;
  // tv.tv_usec = 1000; // 1 millisecond
  // setsockopt(fSr, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  if (1) {
      // https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 1000; // 1 millisecond
      if (setsockopt(fSr /*rcv_sock*/, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0) {
        perror("Error in setting up time out");
      }
  }

#endif
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {

  fMutex.lock();
  fAbort = true;
  fCondition.wakeOne();
  fMutex.unlock();

  //rpc  fRpcThread->quit();
  //rpc  fRpcThread->wait();

  wait();
#ifdef PI
  shutDown();
#endif
}

// ----------------------------------------------------------------------
void driveHardware::sentFromServer(const QString &result) {
  cout << "driveHardware::sentFromServer() -> " << result.toStdString() << "<-" << endl;
  emit signalText(result);
}


// ----------------------------------------------------------------------
void  driveHardware::getMessage(std::string x) {
  QString aline(QString::fromStdString(x));
  emit signalText(aline);

  // -- do something else eventually
}


// ----------------------------------------------------------------------
void  driveHardware::printToGUI(std::string x) {
  QString aline(QString::fromStdString(x));
  emit signalText(aline);
}

// ----------------------------------------------------------------------
//void driveHardware::updateHwDisplay() {
//  cout << "driveHardware::updateHwDisplay() " << endl;
//  emit signalSomething(1);
//}


// ----------------------------------------------------------------------
void driveHardware::runPrintout(int reg, float val) {
  cout << "driveHardware::runPrintout() " << endl;
  QMutexLocker locker(&fMutex);

  this->fCANReg = reg; // ??
  this->fCANVal = val; // ??

  if (!isRunning()) {
      start(LowPriority);
    } else {
      fRestart = true;
      fCondition.wakeOne();
    }

}


// ----------------------------------------------------------------------
void driveHardware::run() {
  cout << "Hallo in run()" << endl;
  int cnt(0);
  while (1) {
      ++cnt;
      std::this_thread::sleep_for(fMilli10);
      readCAN();
      if (cnt%100 == 1) {
          cout << "Hallo in run(), cnt = " << cnt
               << " nframes = " << fCanMsg.nFrames()
               << endl;

          // -- read all parameters from CAN
          fMutex.lock();
          // readAllParamsFromCAN();
          readAllParamsFromCANPublic();
          fMutex.unlock();

          // -- do something with the results
          emit updateHwDisplay();
          dumpCSV();

        }
        if (cnt%150 == 1) fCanMsg.clearFrames();

#ifdef PI
      //entertainFras();
#endif
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

// ----------------------------------------------------------------------
int driveHardware::getTECRegisterIdx(std::string rname) {
  return fTECData[1].getIdx(rname);
}


// ----------------------------------------------------------------------
void  driveHardware::setTECParameter(float par) {
  fTECParameter = par;
  //  printf("driveHardware::setTECParameter = %f\n", fTECParameter);
  QString aline = QString("driveHardware::setTECParameter = ") + QString::number(par, 'f', 2);
  cout << aline.toStdString() << endl;

  emit signalText(aline);
}


// ----------------------------------------------------------------------
void driveHardware::shutDown() {
  // FIXME: do a poff for all TEC controllers. set all relais outputs to 0.
#ifdef PI
  close(fSw);
#endif
}


// ----------------------------------------------------------------------
void driveHardware::readCAN(int nreads) {
  //fMutex.lock();
#ifdef PI
  int nbytes(0);

  bool DBX(false);
  if (nreads > 1) DBX = true;

  static int cntCAN(0);

  char data[4] = {0, 0, 0, 0};

  unsigned int idata(0);
  float fdata(0.0);

  while (nreads > 0) {
    if (DBX) cout << "read(fSr, &fFrameR, sizeof(fFrameR)) ... nreads = " << nreads << endl;
    nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
    if (DBX) cout << "  nbytes = " << nbytes << endl;

    // -- this is an alternative to 'read()'
    //  socklen_t len = sizeof(fAddrR);
    //  nbytes = recvfrom(fSr, &fFrameR, sizeof(fFrameR), 0, (struct sockaddr*)&fAddrR, &len);

    if (nbytes > -1) {
      if (1) {
        printf("can_id = 0x%3X data[%d] = ", fFrameR.can_id, fFrameR.can_dlc);
        for (int i = 0; i < fFrameR.can_dlc; ++i) printf("%02X ", fFrameR.data[i]);
        cout << endl;

        canFrame f(fFrameR.can_id, fFrameR.can_dlc, fFrameR.data);
        fCanMsg.addFrame(f);
      }
    }
    --nreads;
  }
#endif
  //fMutex.unlock();
  return;
}


// ----------------------------------------------------------------------
void driveHardware::readCANmessage() {
  fCANReadIntVal   += 1;
  fCANReadFloatVal += 0.1;
#ifdef PI
  int nbytes(0);

  // -- send read request
  sendCANmessage();

  bool DBX(false);
  int itec = 0;
  int ireg = 0;

  static int cntCAN(0);

  char data[4] = {0, 0, 0, 0};

  unsigned int idata(0);
  float fdata(0.0);

  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));

  // -- this is an alternative to 'read()'
  //  socklen_t len = sizeof(fAddrR);
  //  nbytes = recvfrom(fSr, &fFrameR, sizeof(fFrameR), 0, (struct sockaddr*)&fAddrR, &len);

  if (nbytes > -1) {

      if (DBX) {
          printf("can_id = 0x%X ncan_dlc = %d \n", fFrameR.can_id, fFrameR.can_dlc);
          int i = 0;
          cout << "data[] = ";
          if (DBX) for (i = 0; i < fFrameR.can_dlc; i++) {
              printf("%3d ", fFrameR.data[i]);
            }
        }

      itec    = fFrameR.can_id & 0xf;
      ireg    = fFrameR.data[0];
      data[0] = fFrameR.data[1];
      data[1] = fFrameR.data[2];
      data[2] = fFrameR.data[3];
      data[3] = fFrameR.data[4];

      if (DBX) cout << " ireg = " << ireg << " (fCANReg = " << fCANReg << ") ";

      memcpy(&fdata, data, sizeof fdata);
      memcpy(&idata, data, sizeof idata);
      if (DBX) {
          printf("float = %f/uint32 = %u", fdata, idata);
          ++cntCAN;
          printf(" (received CAN message %d)", cntCAN);
        }
      if (DBX) cout << endl;

      stringstream sbla; sbla << "CAN read canid = " << hex << fFrameR.can_id
                              << " tec = " << itec
                              << " reg = 0x"  << hex << ireg
                              << " value = " << fdata;
      if (DBX) cout << "sbla: " << sbla.str() << endl;
      fLOG(INFO, sbla.str());


      fCANReadIntVal = idata;
      fCANReadFloatVal = fdata;
    }
#endif

  return;
}





// ----------------------------------------------------------------------
void driveHardware::sendCANmessage() {
 //fMutex.lock();

#ifdef PI
  int itec = 0;
  itec = fCANId & 0xf;
  if (1) cout << "sendCANmessage() TEC " << itec << endl;

  if ((itec > 0) && (0 == fActiveTEC[itec])) {
      if (0) cout << "TEC " << itec <<  " not active, skipping" << endl;
      return;
  }

  char data[4] = {0, 0, 0, 0};
  fFrameW.can_id = fCANId;
  int dlength(0), command(0);
  if (0x0 == ((0x0f0 & fCANId)>>4)) {
      // -- x0x is command access and no subsequent 4 bytes are required.
      //    fCANReg indicates the command type
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

  if (1) {
      if (1 == command) {
          cout << "   sendCANmessage: canid = " << fCANId << " cmd = " << fCANReg
               << endl;
        } else {
          cout << "   canid = " << fCANId << " reg = " << fCANReg
               << " value = " << fCANVal
               << " dlength = " << dlength
               << endl;

        }
      printf("    can_id  = 0x%X (from sendCANmessage())\n", fFrameW.can_id);
      printf("    can_dlc = %d\n", fFrameW.can_dlc);

      for (int i = 0; i < fFrameW.can_dlc; ++i) {
          printf("    data[%d] = %2x/%3d\r\n", i, fFrameW.data[i], fFrameW.data[i]);
        }
    }
  //6.Send message
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
  if (nbytes != sizeof(fFrameW)) {
      printf("    Send Error frame[0]!\r\n");
    }

  // -- this is required to absorb the write request from fSr
  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));

#endif
  //fMutex.unlock();
}


// ----------------------------------------------------------------------
void driveHardware::entertainFras() {
#ifdef PI
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
#endif
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
  unsigned int canid = 0x40;
#ifdef PI
  fFrameW.can_id = canid;
  int dlength(1);
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fValveMask;
#endif
  stringstream sbla; sbla << "talkToFras "
                          << " reg = 0x"  << hex << canid
                          << " data = " << fCANVal;
  cout << "sbla: " << sbla.str() << endl;
  fLOG(INFO, sbla.str());

  // -- Send message
#ifdef PI
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
  if (nbytes != sizeof(fFrameW)) {
      printf("Send Error frame[0]!\r\n");
    }

  //  fMutex.unlock();
#endif
}


// ----------------------------------------------------------------------
void driveHardware::toggleFras(int imask) {

  int old = fValveMask;

  fValveMask = old xor imask;

  cout << "toggleFras old = " << hex << old
       << " imask = " << imask
       << " fValveMask = " << fValveMask
       << endl;
  //  fMutex.lock();

  //            TEC:   ssP'..tt'aaaa
  //           FRAS:   0aa'aaaa'akkk
  //  fFrameW.can_id = 000'0100'0000 -> 0x040 for process
  //  fFrameW.can_id = 000'0100'0001 -> 0x041 for service
  //  fFrameW.can_id = 000'0100'0010 -> 0x042 for control

  unsigned int canid = 0x40;
#ifdef PI
  fFrameW.can_id = canid;
  int dlength(1);
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fValveMask;
#endif
  stringstream sbla; sbla << "talkToFras "
                          << " reg = 0x"  << hex << canid
                          << " data = " << fCANVal;
  cout << "sbla: " << sbla.str() << endl;
  fLOG(INFO, sbla.str());

  // -- Send message
#ifdef PI
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
  if (nbytes != sizeof(fFrameW)) {
      printf("Send Error frame[0]!\r\n");
    }
#endif
  //  fMutex.unlock();

}


// ----------------------------------------------------------------------
float driveHardware::getTECRegister(int itec, std::string regname) {
  if (fTECData.find(itec) == fTECData.end()) {
      return -1.;
    }
  if (fTECData[itec].reg.find(regname) != fTECData[itec].reg.end()) {
      return fTECData[itec].reg[regname].value;
    } else {
      return -3.;
    }
}


// ----------------------------------------------------------------------
void  driveHardware::turnOnTEC(int itec) {

  if (0 == fActiveTEC[itec]) {
      cout << "TEC " << itec <<  " not active, skipping" << endl;
      return;
    }

  printf("driveHardware::turnOnTEC(%d)\n", itec);

  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 1; // Power_On
  fCANVal = fTECParameter;
  printf(" (2) send CMD with register %d, canID = %x\n", fCANReg, fCANId);
  sendCANmessage();
}


// ----------------------------------------------------------------------
void  driveHardware::turnOffTEC(int itec) {

  if (0 == fActiveTEC[itec]) {
      cout << "TEC " << itec <<  " not active, skipping" << endl;
      return;
    }

  printf("driveHardware::turnOffTEC(%d)\n", itec);

  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 2; // Power_Off
  fCANVal = fTECParameter;
  printf(" send CMD with register %d, canID = %x\n", fCANReg, fCANId);
  sendCANmessage();
}


// ----------------------------------------------------------------------
float driveHardware::getTECRegisterFromCAN(int itec, std::string regname) {

  if (itec > 0) {
    if (0 == fActiveTEC[itec]) {
      return -99.;
    }
  }

  if (itec > 0) {
    fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_READ);
    fCANReg = fTECData[itec].getIdx(regname);
  } else {
    fCANId = (0 | CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_READ);
    fCANReg = fTECData[1].getIdx(regname);
  }
  // -- send read request
  sendCANmessage();
  std::this_thread::sleep_for(fMilli10);
  if (itec > 0) {
    readCAN();
    fCANReadFloatVal = fCanMsg.getFloat(itec, fCANReg);
    cout << "  obtained for tec = " << itec
         << " register = "<< regname
         << " value = " << fCANReadFloatVal
         << " nframes = " << fCanMsg.nFrames()
         << endl;
    //  readCANmessage();

    return fCANReadFloatVal;
  } else {
    readCAN(fNActiveTEC);
    return -97;
  }
  return -96;
}


// ----------------------------------------------------------------------
void driveHardware::setTECRegister(int itec, std::string regname, float value) {
  fTECData[itec].reg[regname].value = value;

  // -- program parameter
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_WRITE);

  fCANReg = fTECData[itec].getIdx(regname);

  fCANVal = value;

  QString aline = QString("write TEC register ") + QString::number(fCANReg, 'd', 0)
      + QString(" value =  ") + QString::number(fCANVal, 'f', 2)
      + QString(", canID = 0x") + QString::number(fCANId, 'x', 0)
      ;

  cout << aline.toStdString() << endl;

  emit signalText(aline);
  sendCANmessage();

}


// ----------------------------------------------------------------------
void driveHardware::initTECData() {
  for (unsigned int itec = 1; itec <=8; ++itec) {
      fTECData.insert(make_pair(itec, initAllTECRegister()));
      // -- just for fun:
      // fTECData[itec].reg["ControlVoltage_Set"].value = -static_cast<float>(itec);
      // fTECData[itec].print();
    }
}


// ----------------------------------------------------------------------
TECData  driveHardware::initAllTECRegister() {
  TECData tdata;

  TECRegister b;
  // -- read/write registers
  b = {1,   "Mode",                 0, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {0.,  "ControlVoltage_Set",   1, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "PID_kp",              2, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "PID_ki",              3, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "PID_kd",              4, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {23.1, "Temp_Set",            5, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "PID_Max",             6, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "PID_Min",             7, 1}; tdata.reg.insert(make_pair(b.name, b));
  // -- read-only registers
  b = {-99., "Temp_W",              8, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Temp_M",              9, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Temp_Diff",          10, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Peltier_U",          11, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Peltier_I",          12, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Peltier_R",          13, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Peltier_P",          14, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Supply_U",           15, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Supply_I",           16, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Supply_P",           17, 2}; tdata.reg.insert(make_pair(b.name, b));
  // -- commands
  b = {-99., "No Command",          0, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Power_On",            1, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Power_Off",           2, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Watchdog",            3, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99., "Alarm",               4, 3}; tdata.reg.insert(make_pair(b.name, b));

  return tdata;
}

// ----------------------------------------------------------------------
void driveHardware::readAllParamsFromCANPublic() {

  string regname("Temp_M");
  cout << "driveHardware::readAllParamsFromCANPublic() read Temp_M" << endl;
  getTECRegisterFromCAN(0, regname);
  int ireg = fTECData[1].getIdx(regname);
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_M"].value = fCanMsg.getFloat(i, ireg);


//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_M"].value = getTECRegisterFromCAN(i, "Temp_M");
//  return;

//  cout << "driveHardware::readAllParamsFromCAN() read ControlVoltage_Set" << endl;
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["ControlVoltage_Set"].value = getTECRegisterFromCAN(i, "ControlVoltage_Set");

//  cout << "driveHardware::readAllParamsFromCAN() read PID_kp" << endl;
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["PID_kp"].value = getTECRegisterFromCAN(i, "PID_kp");

//  cout << "driveHardware::readAllParamsFromCAN() read PID_ki" << endl;
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["PID_ki"].value = getTECRegisterFromCAN(i, "PID_ki");

//  cout << "driveHardware::readAllParamsFromCAN() read PID_kd" << endl;
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["PID_kd"].value = getTECRegisterFromCAN(i, "PID_kd");

//  cout << "driveHardware::readAllParamsFromCAN() read all the rest" << endl;
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_W"].value = getTECRegisterFromCAN(i, "Temp_W");
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_Set"].value = getTECRegisterFromCAN(i, "Temp_Set");
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Peltier_I"].value = getTECRegisterFromCAN(i, "Peltier_I");
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Peltier_R"].value = getTECRegisterFromCAN(i, "Peltier_R");
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Peltier_P"].value = getTECRegisterFromCAN(i, "Peltier_P");

//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Supply_U"].value = getTECRegisterFromCAN(i, "Supply_U");
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Supply_I"].value = getTECRegisterFromCAN(i, "Supply_I");
//  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Supply_P"].value = getTECRegisterFromCAN(i, "Supply_P");

}


// ----------------------------------------------------------------------
void driveHardware::readAllParamsFromCAN() {

  cout << "driveHardware::readAllParamsFromCAN() read Temp_M" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_M"].value = getTECRegisterFromCAN(i, "Temp_M");
  return;

  cout << "driveHardware::readAllParamsFromCAN() read ControlVoltage_Set" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["ControlVoltage_Set"].value = getTECRegisterFromCAN(i, "ControlVoltage_Set");

  cout << "driveHardware::readAllParamsFromCAN() read PID_kp" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["PID_kp"].value = getTECRegisterFromCAN(i, "PID_kp");

  cout << "driveHardware::readAllParamsFromCAN() read PID_ki" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["PID_ki"].value = getTECRegisterFromCAN(i, "PID_ki");

  cout << "driveHardware::readAllParamsFromCAN() read PID_kd" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["PID_kd"].value = getTECRegisterFromCAN(i, "PID_kd");

  cout << "driveHardware::readAllParamsFromCAN() read all the rest" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_W"].value = getTECRegisterFromCAN(i, "Temp_W");
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_Set"].value = getTECRegisterFromCAN(i, "Temp_Set");
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Peltier_I"].value = getTECRegisterFromCAN(i, "Peltier_I");
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Peltier_R"].value = getTECRegisterFromCAN(i, "Peltier_R");
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Peltier_P"].value = getTECRegisterFromCAN(i, "Peltier_P");

  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Supply_U"].value = getTECRegisterFromCAN(i, "Supply_U");
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Supply_I"].value = getTECRegisterFromCAN(i, "Supply_I");
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Supply_P"].value = getTECRegisterFromCAN(i, "Supply_P");

}


// ----------------------------------------------------------------------
void driveHardware::dumpCSV() {
  stringstream output;

  output  << timeStamp();
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["ControlVoltage_Set"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Temp_Set"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["PID_kp"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["PID_ki"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["PID_kd"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Temp_M"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Temp_W"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Peltier_I"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Peltier_R"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Peltier_P"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Supply_U"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Supply_I"].value;
  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Supply_P"].value;

  string sout = output.str();
  fCsvFile << sout << endl;
}

// ----------------------------------------------------------------------
string driveHardware::timeStamp(bool filestamp) {
  char buffer[11];
  time_t t;
  time(&t);
  tm r;
  strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);

  tm *ltm = localtime(&t);
  int year  = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day   = ltm->tm_mday;
  int hour  = ltm->tm_hour;
  int min   = ltm->tm_min;
  int sec   = ltm->tm_sec;
  std::stringstream result;
  if (filestamp) {
      result << year
             << std::setfill('0') << std::setw(2) << month
             << std::setfill('0') << std::setw(2) << day
             << ":"
             << std::setfill('0') << std::setw(2) << hour
             << std::setfill('0') << std::setw(2) << min
             << std::setfill('0') << std::setw(2) << sec
             << std::setfill('0') << std::setw(3) << ((long)tv.tv_usec / 1000);
      return result.str();
    }
  result << year << "/" << std::setfill('0') << std::setw(2) << month << "/" << day << " "
         << buffer << "." << std::setfill('0') << std::setw(3) << ((long)tv.tv_usec / 1000);
  return result.str();
}
