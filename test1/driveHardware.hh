#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <iostream>

#include <QtCore/QObject>

#include <fstream>
#include <time.h>


#include "CANmessage.hh"

#include "TECData.hh"
#include "tLog.hh"

#ifdef PI
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

const unsigned int CANBUS_SHIFT   = 0x200;
const unsigned int CANBUS_PRIVATE = 0x100;
const unsigned int CANBUS_PUBLIC  = 0x000;

const unsigned int CANBUS_CMD     = 0x000;
const unsigned int CANBUS_READ    = 0x010;
const unsigned int CANBUS_WRITE   = 0x020;

const unsigned int CANBUS_TECSEND = 0x040;
const unsigned int CANBUS_TECREC  = 0x000;


// ----------------------------------------------------------------------
class driveHardware: public QObject {
  Q_OBJECT

public:
  driveHardware(tLog &x);
  ~driveHardware();

  void  shutDown();

  void dumpCSV();
  void evtHandler();

  std::string timeStamp(bool filestamp = true);
  std::string tStamp() {return timeStamp(false);}

  void  readCAN(int nreads = 1);
  void  sendCANmessage();
  void  readCANmessage();
  void  parseCAN();

  void parseIoMessage();

  // -- controlling the FRAS/valve(s)
  void  talkToFras();
  void  entertainFras();

  // -- controlling the TEC
  void  setTECParameter(float par); // ???

  void  setId(int x);
  void  setRegister(int x);
  void  setValue(float x);
  int   getId();
  int   getRegister();
  float getValue();

  // -- environmental data
  void  readSHT85();
  float getTemperature();
  float getRH();
  float getDP();
  int   getRunTime();
  int   getNCANbusErrors();
  int   getNI2CErrors() {return fI2CErrorCounter;}
  int   getRunCnt() {return fRunCnt;}

  // -- simply returns the value stored in fTECData
  float getTECRegister(int itec, std::string regname);
  int   getTECRegisterIdx(std::string rname);

  // -- read from and write to CAN
  float getTECRegisterFromCAN(int itec, std::string regname);
  void  setTECRegister(int itec, std::string regname, float value);

  // -- fill 'all' parameters for dumping into csv or for refreshing the GUI
  void readAllParamsFromCAN();
  // -- same as above, but with "public" broadcast
  void readAllParamsFromCANPublic();

  // -- AFTER readCANmessage() these can be used to get the relevant value
  float getCANReadFloatVal() {return fCANReadFloatVal;}
  int   getCANReadIntVal() {return fCANReadIntVal;}


public slots:
  void  sentFromServer(QString msg);
  void  toggleFras(int imask);
  void  turnOnTEC(int itec);
  void  turnOffTEC(int itec);
  void  doRun();

signals:
  void  signalSomething(int x);
  void  signalText(QString msg);
  void  signalSendToServer(QString msg);
//REMOVE  void  startServer();
  void  signalUpdateHwDisplay();

protected:
  void        initTECData();
  TECData     initAllTECRegister();
  int         diff_ms(timeval t1, timeval t2);

private:
  tLog&   fLOG;
  QMutex fMutex;
  QWaitCondition fCondition;

  CANmessage fCanMsg;

  std::string fIoMessage;

  bool    fRestart;
  bool    fAbort;
  int     fCANId;
  int     fCANReg;
  float   fCANVal;
  float   fTECParameter;
  int     fValveMask; 
  QString fDateAndTime;

  std::map<int, int> fActiveTEC;
  int fNActiveTEC;

  int     fCANReadIntVal;
  float   fCANReadFloatVal;

  std::string fCsvFileName;
  std::ofstream fCsvFile;

  int fI2CErrorCounter;

  // -- timing and wall-clock ticks (or so)
  std::chrono::milliseconds fMilli5, fMilli10, fMilli100;
  struct timeval ftvStart;
  int    fRunCnt;

  // -- access and data from SHT85
  char fSHT85Data[6], fSHT85Config[2];
  int  fSHT85File;
  float fSHT85Temp, fSHT85RH, fSHT85DP;

#ifdef PI
  int    fSw; 
  struct sockaddr_can fAddrW;
  struct ifreq fIfrW;
  struct can_frame fFrameW;

  int    fSr; 
  struct sockaddr_can fAddrR;
  struct ifreq fIfrR;
  struct can_frame fFrameR;

#endif

  // -- all the registers, one element per TEC
  // -- this is a map instead of a vector to avoid the mismatch between '0' and '1'
  std::map<int, TECData> fTECData;

};

#endif // DRIVEHARDWARE_H
