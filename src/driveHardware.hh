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
  driveHardware(tLog &x, int verbose);
  ~driveHardware();

  void  shutDown();

  void dumpCSV();
  void dumpMQTT(int all = 0);
  void evtHandler();
  void ensureSafety();
  void doWarning(std::string errmsg, bool nothing = false);
  void doAlarm(std::string s);
  void lighting(int imode = 0);
  void breakInterlock();
  void resetInterlock();

  std::string timeStamp(bool filestamp = true);
  std::string tStamp() {return timeStamp(false);}

  void  readCAN(int nreads = 1, bool setMutex = true);
  void  sendCANmessage(bool setMutex = true);
  void  parseCAN();

  void parseIoMessage();

  // -- return true if either s1 or s2 in fIoMessage
  bool findInIoMessage(std::string &s1, std::string &s2, std::string &s3);
  void answerIoGet(std::string &what);
  void answerIoSet(std::string &what);
  void answerIoCmd();

  // -- controlling the FRAS/valve(s)
  void  entertainFras();
  bool  getStatusValve0() {return ((fRelaisMask & 1) == 1);}
  bool  getStatusValve1() {return ((fRelaisMask & 2) == 2);}
  bool  getStatusValve()  {return (getStatusValve0() && getStatusValve1());}
  bool  getStatusFan() {return ((fRelaisMask & 4) == 4);}
  bool  getStatusLVInterlock() {return ((fRelaisMask & 8) == 8);}
  int   getLidStatus() {return fLidStatus;}
  int   getInterlockStatus() {return fInterlockStatus;}
  int   getThrottleStatus() {return fThrottleStatus;}
  void  turnOnValve(int i); // i = 0 or 1
  void  turnOffValve(int i); // i = 0 or 1
  void  turnOnFan();
  void  turnOffFan();
  void  turnOnLV();
  void  turnOffLV();
  bool  anyTECRunning();
  void  checkFan();
  void  checkLid();
  void  throttleN2();
  std::string getStatusString() {return fStatusString;}
  std::string getHostname() {return fHostName;}
  void  checkDiskspace();
  
  // -- controlling the TEC
  void  setTECParameter(float par); // ???
  void  entertainTECs();

  void   setId(int x);
  void   setRegister(int x);
  void   setValue(float x);
  int    getId();
  int    getRegister();
  float  getValue();
  int    getSWVersion(int itec = 0);
  char   crc(char *data, size_t len);

#ifdef UZH
  int   readI2C();
#endif

  // -- environmental data
  void    readSHT85();
  void    readVProbe(int ipos);
  void    readFlowmeter();
  float   getTemperature();
  float   getRH();
  float   getDP();
  float   calcDP(int mode = 0);

  int   getRunTime();
  int   getNCANbusErrors() {return fCANErrorCounter;}
  int   redCANErrors() {return fCANErrorCounter - fCANErrorOld;}
  int   getNI2CErrors() {return fI2CErrorCounter;}
  int   redI2CErrors() {return fI2CErrorCounter - fI2CErrorOld;}
  int   getRunCnt() {return fRunCnt;}
  int   getFreeDisk() {checkDiskspace(); return fFreeDiskspace;}
  int   getFlowSwitchStatus() {return fFlowMeterStatus;}
  
  // -- simply returns the value stored in fTECData
  float getTECRegister(int itec, std::string regname);
  int   getTECRegisterIdx(std::string rname);

  // -- read from and write to CAN
  float getTECRegisterFromCAN(int itec, std::string regname);
  void  setTECRegister(int itec, std::string regname, float value);

  // -- same as above, but with "public" broadcast
  void  readAllParamsFromCANPublic();

  // -- AFTER readCANmessage() these can be used to get the relevant value
  float getCANReadFloatVal() {return fCANReadFloatVal;}
  int   getCANReadIntVal() {return fCANReadIntVal;}

  // -- FLASH save/load
  void loadFromFlash();
  void saveToFlash();

public slots:
  void  sentFromServer(QString msg);
  void  toggleFras(int imask);
  void  stopOperations(int);
  void  turnOnTEC(int itec);
  void  turnOffTEC(int itec);
  void  doRun();

signals:
  void  signalSomething(int x);
  void  signalSetBackground(QString name, QString color);
  void  signalText(QString msg);
  void  signalSendToServer(QString msg);
  void  signalSendToMonitor(QString msg);
  //REMOVE  void  startServer();
  void  signalUpdateHwDisplay();
  void  signalAlarm(int);
  void  signalKillSiren();


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
  int     fVerbose;
  int     fCANId;
  int     fCANReg;
  float   fCANVal;
  float   fTECParameter;
  int     fRelaisMask;
  QString fDateAndTime;

  std::map<int, int> fActiveTEC;
  int fNActiveTEC;

  int     fCANReadIntVal;
  float   fCANReadFloatVal;

  std::string fCsvFileName;
  std::ofstream fCsvFile;

  int fCANErrorCounter{0}, fCANErrorOld{0};
  int fI2CErrorCounter{0}, fI2CErrorOld{0};

  // -- timing and wall-clock ticks (or so)
  std::chrono::milliseconds fMilli5, fMilli10, fMilli20, fMilli100;
  struct timeval ftvStart;
  int    fRunCnt;

  // -- access and data from SHT85
  char fSHT85Data[6], fSHT85Config[2];
  float fSHT85Temp, fSHT85RH, fSHT85DP;

  // -- data from VProbe
  std::string fVprobeVoltages;

  // -- lid status (read out from TEC7 (of 8)
  //    1 closed and locked (TEC7 Temp_W > 4000)
  //    0 closed            (TEC7 0 < Temp_W < 4000)
  //    -1 open             (TEC7 Temp_W < 0)
  int fLidStatus, fOldLidStatus;
  double fLidReading; 
  int fInterlockStatus;
  
#ifdef PI
  int    fSw;
  struct sockaddr_can fAddrW;
  struct ifreq fIfrW;
  struct can_frame fFrameW;

  int    fSr;
  struct sockaddr_can fAddrR;
  struct ifreq fIfrR;
  struct can_frame fFrameR;

  int    fPiGPIO;
#endif

#ifdef UZH
  bool fBusyDisarm;
#endif
  
  // -- all the registers, one element per TEC
  // -- this is a map instead of a vector to avoid the mismatch between '0' and '1'
  std::map<int, TECData> fTECData;

  // -- keep alarm state
  int fAlarmState;

  std::string fStatusString, fHostName;
  int fFreeDiskspace, fTrafficRed, fTrafficYellow, fTrafficGreen;
  int fStopOperations, fFlowMeterStatus, fThrottleStatus;


  const double SAFETY_DPMARGIN = 2.;
  
  // -- reset to 35 if flowmeter present
  double MAX_TEMP = 25.; 
  // -- various temperatures
  double SAFETY_MAXSHT85TEMP{MAX_TEMP};
  double SAFETY_MAXTEMPW{MAX_TEMP};
  double SAFETY_MAXTEMPM{MAX_TEMP};
  double SHUTDOWN_TEMP{MAX_TEMP};
  
};

#endif // DRIVEHARDWARE_H
