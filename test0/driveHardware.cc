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
  fFrequency = 0;
  fOffset    = 0;
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
  memset(&fFrame, 0, sizeof(struct can_frame));

  fS = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fS < 0) {
    perror("socket PF_CAN failed");
    return;
  }
    
  strcpy(fIfr.ifr_name, "can0");
  int ret = ioctl(fS, SIOCGIFINDEX, &fIfr);
  if (ret < 0) {
    perror("ioctl failed");
    return;
  }
  
  fAddr.can_family = AF_CAN;
  fAddr.can_ifindex = fIfr.ifr_ifindex;
  ret = bind(fS, (struct sockaddr *)&fAddr, sizeof(fAddr));
  if (ret < 0) {
    perror("bind failed");
    return;
  }
    
  //4.Disable filtering rules, do not receive packets, only send
  //  setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  
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
void driveHardware::runPrintout(int freq, int off) {
  QMutexLocker locker(&fMutex);
  this->fFrequency = freq;
  this->fOffset    = off;

  if (!isRunning()) {
    start(LowPriority);
  } else {
    fRestart = true;
    fCondition.wakeOne();
  }

}


// ----------------------------------------------------------------------
void driveHardware::run() {

  while (1) {
    fMutex.lock();
    std::chrono::milliseconds sec(1000/this->fFrequency);
    int cn = fOffset++;
    // -- keep for possible debugging but remove for long-term testing on pi
    if (0) {
        stringstream print;
        print << "countUp: " << cn  << " fFrequency = " << fFrequency;
        fLOG(DEBUG, print.str());
    }

#ifdef PI
    int cnt(0), nbytes(0); 
    char data[4]; 
    unsigned int idata(0);
    float fdata(0.0);
    while(1) {
      while(1) {
        nbytes = read(fS, &fFrame, sizeof(fFrame));
        if(nbytes > 0) {
          printf("can_id = 0x%X\r\ncan_dlc = %d \r\n", fFrame.can_id, fFrame.can_dlc);
          int i = 0;
          
          for(i = 0; i < fFrame.can_dlc; i++) {
            printf("data[%d] = %2x/%3d\r\n", i, fFrame.data[i], fFrame.data[i]);
          }
          
          data[0] = fFrame.data[1];
          data[1] = fFrame.data[2];
          data[2] = fFrame.data[3];
          data[3] = fFrame.data[4];
          
          memcpy(&fdata, data, sizeof fdata); 
          memcpy(&idata, data, sizeof idata); 
          printf("float = %f/uint32 = %u\r\n", fdata, idata);
          ++cnt;
          break;
        }
      }
      
      printf("received message %d\r\n", cnt);
    }

#endif

    fMutex.unlock();
    //    sleep(1./fFrequency);
    std::this_thread::sleep_for(sec);

  }

}


// ----------------------------------------------------------------------
void driveHardware::setFrequency(int freq) {
  fFrequency = freq;
}

// ----------------------------------------------------------------------
void driveHardware::setOffset(int oset) {
  fOffset = oset;
}

// ----------------------------------------------------------------------
int driveHardware::getFrequency() {
  return fFrequency;
}

// ----------------------------------------------------------------------
int driveHardware::getOffset() {
  return fOffset;
}


#ifdef PI
void driveHardware::shutDown() {
  close(fS);
}

void driveHardware::sendCANmessage(unsigned int id, unsigned int reg, char bytes[4]) {
}

void driveHardware::readCANmessage(unsigned int id, unsigned int reg, char bytes[4]) {
  bytes[0] = 0; 
  bytes[1] = 0; 
  bytes[2] = 0; 
  bytes[3] = 0; 
  return;
}
#endif
