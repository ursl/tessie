#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <net/if.h>
#include <sys/ioctl.h>

#include <fcntl.h>


#ifdef PI
#include <linux/i2c-dev.h>
#endif

// -- i2c address of SHT85 sensor
#define I2C_ADDR 0x44

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

  fI2CErrorCounter = 0;

  gettimeofday(&ftvStart, 0);
  fMilli5   = std::chrono::milliseconds(5);
  fMilli10  = std::chrono::milliseconds(10);
  fMilli100 = std::chrono::milliseconds(100);

  fCsvFileName = "tessie.csv";
  fLOG(INFO, stringstream("open" + fCsvFileName).str());
  fCsvFile.open(fCsvFileName, ios_base::app);

  for (unsigned int itec = 1; itec <= 4; ++itec) {
    fActiveTEC.insert(make_pair(itec, 0));
  }
  for (unsigned int itec = 5; itec <= 8; ++itec) {
    fActiveTEC.insert(make_pair(itec, 1));
  }
  fNActiveTEC = 4;

  initTECData();

  fSHT85Temp = -99.;
  fSHT85RH   = -99.;
  fSHT85DP   = -99.;
  for (int i = 0; i < 6; ++i) {
    fSHT85Data[i] = 0;
  }

  fSHT85Config[0] = 0x24;   // MSB
  fSHT85Config[1] = 0x00;   // LSB

#ifdef PI

  // -- create I2C bus
  cout << "Open I2C bus for SHT85" << endl;

  char *bus = "/dev/i2c-0";
  if ((fSHT85File = open(bus, O_RDWR)) < 0) {
    cout << "Failed to open the bus." << endl;
    exit(1);
  } else {
    cout << "I2C bus opened with fSHT85File = " << fSHT85File << endl;
  }

  // -- get I2C device, SHT85 I2C address is 0x44
  ioctl(fSHT85File, I2C_SLAVE, I2C_ADDR);

  readSHT85();
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
      tv.tv_usec = 10; // 0.1 millisecond
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
  struct timeval tvOld, tvNew;
  gettimeofday(&tvOld, 0);

  while (1) {
    ++cnt;
    std::this_thread::sleep_for(fMilli5);
    readCAN();
    gettimeofday(&tvNew, 0);
    int tdiff = diff_ms(tvNew, tvOld);
    if (tdiff > 1000.) {
      tvOld = tvNew;
      if (0) cout << tStamp() << " readAllParamsFromCANPublic(), tdiff = " << tdiff << endl;
      readSHT85();

      // -- read all parameters from CAN
      fMutex.lock();
      readAllParamsFromCANPublic();
      fMutex.unlock();

      // -- do something with the results
      emit updateHwDisplay();
      dumpCSV();

      // -- make sure there is no alarm before clearing
      parseCAN();
      // -- print errors (if present) accumulated in CANmessage
      if (fCanMsg.nErrors() > 0) {
        deque<string> errs = fCanMsg.getErrors();
        while (errs.size() > 0) {
          fLOG(WARNING, errs.front());
          errs.pop_front();
        }
      }
      fCanMsg.clearAllFrames();
    }

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
  close(fSr);
  close(fSHT85File);
#endif
}


// ----------------------------------------------------------------------
void driveHardware::readCAN(int nreads) {
  //fMutex.lock();
#ifdef PI
  int nbytes(0);

  bool DBX(false);
  // if (nreads > 1) DBX = true;

  static int cntCAN(0);

  char data[4] = {0, 0, 0, 0};

  unsigned int idata(0);
  float fdata(0.0);

  ++nreads;
  while (nreads > 0) {
    if (DBX) cout << "read(fSr, &fFrameR, sizeof(fFrameR)) ... nreads = " << nreads << endl;
    nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
    if (DBX) cout << "  nbytes = " << nbytes << endl;

    // -- this is an alternative to 'read()'
    //  socklen_t len = sizeof(fAddrR);
    //  nbytes = recvfrom(fSr, &fFrameR, sizeof(fFrameR), 0, (struct sockaddr*)&fAddrR, &len);

    if (nbytes > -1) {
      if (1) {
        canFrame f(fFrameR.can_id, fFrameR.can_dlc, fFrameR.data);
        fCanMsg.addFrame(f);
      }
    }
    --nreads;
  }
#endif
  parseCAN();
  //fMutex.unlock();
  return;
}


// ----------------------------------------------------------------------
void driveHardware::parseCAN() {
  if (fCanMsg.getFRASMessage() > 0) {
    entertainFras();
  }

  if (fCanMsg.getAlarm() > 0) {
    cout << "received alarm from CAN bus, do something!" << endl;
  }

}


// ----------------------------------------------------------------------
// DECREPIT
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
  if (0) cout << "sendCANmessage() TEC " << itec << endl;

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

  if (0) {
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
  //  fMutex.lock();

  //            TEC:   ssP'..tt'aaaa
  //           FRAS:   0aa'aaaa'akkk
  //  fFrameW.can_id = 000'0100'0000 -> 0x040 for process
  //  fFrameW.can_id = 000'0100'0001 -> 0x041 for service
  //  fFrameW.can_id = 000'0100'0010 -> 0x042 for control

#ifdef PI
  unsigned int canid = 0x40;
  fFrameW.can_id = canid;
  int dlength(1);
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fValveMask;
#endif

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
    if (0) cout << "  obtained for tec = " << itec
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
  b = {-99., "PowerState",         18, 2}; tdata.reg.insert(make_pair(b.name, b));
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
  // -- what to read
  vector<string> regnames = {"Temp_M",
                            "ControlVoltage_Set",
                            "PID_kp",
                            "PID_ki",
                            "PID_kd",
                            "Temp_W",
                            "Temp_Set",
                             "Peltier_I",
                             "Peltier_R",
                             "Peltier_P",
                             "Supply_U",
                             "Supply_I",
                             "Supply_P"
                            };
  for (unsigned int ireg = 0; ireg < regnames.size(); ++ireg) {
  //for (unsigned int ireg = 0; ireg < 1; ++ireg) {
    getTECRegisterFromCAN(0, regnames[ireg]);
    if (0) cout << "  " << tStamp() << " reading broadcast "<< regnames[ireg] << endl;
    int regIdx = fTECData[1].getIdx(regnames[ireg]);
    for (int i = 1; i <= 8; ++i) {
      if (0 == fActiveTEC[i]) continue;
      fTECData[i].reg[regnames[ireg]].value = fCanMsg.getFloat(i, regIdx);
    }
  }

  // -- read PowerState
  getTECRegisterFromCAN(0, "PowerState");
  int regIdx = fTECData[1].getIdx("PowerState");
  for (int i = 1; i <= 8; ++i) {
    if (0 == fActiveTEC[i]) continue;
    fTECData[i].reg["PowerState"].value = fCanMsg.getInt(i, regIdx);
  }
}


// ----------------------------------------------------------------------
void driveHardware::readAllParamsFromCAN() {

  cout << "driveHardware::readAllParamsFromCAN() read Temp_M" << endl;
  for (int i = 1; i <= 8; ++i) fTECData[i].reg["Temp_M"].value = getTECRegisterFromCAN(i, "Temp_M");
  //return;

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
  output << fSHT85Temp << "," << fSHT85RH << "," << fSHT85DP;
  for (int i = 1; i <= 1; ++i) {
    output << "," << fTECData[i].reg["Temp_W"].value;
  }

  for (int i = 1; i <= 8; ++i) {
    if (fActiveTEC[i]) output << "," << fTECData[i].reg["ControlVoltage_Set"].value;
  }
  for (int i = 1; i <= 8; ++i) {
    if (fActiveTEC[i]) output << "," << fTECData[i].reg["Temp_Set"].value;
  }

//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["PID_kp"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["PID_ki"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["PID_kd"].value;

  for (int i = 1; i <= 8; ++i) {
    if (fActiveTEC[i]) output << "," << fTECData[i].reg["Temp_M"].value;
  }

//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Peltier_I"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Peltier_R"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Peltier_P"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Supply_U"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Supply_I"].value;
//  for (int i = 1; i <= 8; ++i) output << "," << fTECData[i].reg["Supply_P"].value;

  fCsvFile <<  output.str() << endl;
}

// ----------------------------------------------------------------------
string driveHardware::timeStamp(bool filestamp) {
  return fLOG.timeStamp(filestamp);
}

// ----------------------------------------------------------------------
void driveHardware::readSHT85() {
#ifdef PI
  // -- send high repeatability measurement command
  //    command msb, command lsb(0x2C, 0x06)
  write(fSHT85File, fSHT85Config, 2);
  std::this_thread::sleep_for(fMilli100);

  // -- read 6 bytes of data
  //    temp msb, temp lsb, temp CRC, humidity msb, humidity lsb, humidity CRC
  if (read(fSHT85File, fSHT85Data, 6) != 6) {
    fLOG(WARNING, "I2C Error: Input/output Error with SHT85");
    ++fI2CErrorCounter;
  } else {
    // -- convert the data
    //double cTemp = (((fSHT85Data[0] * 256) + fSHT85Data[1]) * 175.0) / 65535.0  - 45.0;
    //double humidity = (((fSHT85Data[3] * 256) + fSHT85Data[4])) * 100.0 / 65535.0;
    double norm = 65535.0;
    double st   = (fSHT85Data[0]<<8) + fSHT85Data[1];
    fSHT85Temp  = (st * 175.0) / norm  - 45.0;

    st          = (fSHT85Data[3]<<8) + fSHT85Data[4];
    fSHT85RH    = (st * 100.0) / norm;

    // -- calculate dew point
    //    https://doi.org/10.1175/BAMS-86-2-225
    double td0 = fSHT85Temp - (100. - fSHT85RH)/5.; // most simple approximation
    fSHT85DP = static_cast<float>(td0);

    // -- print
    if (0) {
      cout << "Temperature in Celsius: " << fSHT85Temp << endl;
      cout << "Relative Humidity:      " << fSHT85RH << endl;
    }
  }
#endif
}

// ----------------------------------------------------------------------
float driveHardware::getTemperature() {
  return fSHT85Temp;
}

// ----------------------------------------------------------------------
float driveHardware::getRH() {
  return fSHT85RH;
}

// ----------------------------------------------------------------------
float driveHardware::getDP() {
  return fSHT85DP;
}

// ----------------------------------------------------------------------
int driveHardware::getRunTime() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  int dt = tv.tv_sec - ftvStart.tv_sec;
  return dt;
}

// ----------------------------------------------------------------------
int driveHardware::getNCANbusErrors() {
  return fCanMsg.nErrors();
}


// ----------------------------------------------------------------------
int driveHardware::diff_ms(timeval t1, timeval t2) {
  return (((t1.tv_sec - t2.tv_sec) * 1000000) +
          (t1.tv_usec - t2.tv_usec))/1000;
}
