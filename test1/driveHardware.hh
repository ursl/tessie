#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <iostream>

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTime>

#include <fstream>

#include "tLog.hh"
//rpc #include "rpcServer.hh"

#ifdef PI
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif


// ----------------------------------------------------------------------
struct TECRegister {
  float       value;
  std::string name;
  uint32_t    idx;
  uint32_t    type; // 0 = unset, 1 = W/R, 2 = R, 3 = C
};


// ----------------------------------------------------------------------
struct TECData {
  int getIdx(std::string regname) {
    // FIXME: This is wrong and always returns 0!
    int idx = reg.find(regname)->second.idx;
    // FIXME: Therefore replace with for loop
    for (std::map<std::string, TECRegister>::iterator it = reg.begin(); it != reg.end(); ++it) {
        if (std::string::npos != it->first.find(regname)) {
            if (0) std::cout << "regname " << regname << " -> idx = " << idx
                             << " second = " << reg.find(regname)->second.name
                             << std::endl;
            return idx;
          }
      }
    return -1;
  }
  void print() {
    for (std::map<std::string, TECRegister>::iterator it = reg.begin(); it != reg.end(); ++it) {
        std::cout << it->first << " idx: " << it->second.idx
                  << " value: " << it->second.value
                  << " type: " << it->second.type
                  << std::endl;
      }
  }
  std::map<std::string, TECRegister> reg;
};


// ----------------------------------------------------------------------
class driveHardware: public QThread {
  Q_OBJECT

public:
  driveHardware(tLog &x, QObject *parent = nullptr);
  ~driveHardware();

  void  runPrintout(int reg, float val);
  void dumpCSV();
  std::string timeStamp(bool filestamp = true);

  void  sendCANmessage();
  void  readCANmessage();

  // -- controlling the FRAS/valve(s)
  void  talkToFras();
  void  toggleFras(int imask);
  void  entertainFras();
  void  shutDown();

  // -- controlling the TEC
  void  setTECParameter(float par); // ???
  void  turnOnTEC(int itec);
  void  turnOffTEC(int itec);

  void  setId(int x);
  void  setRegister(int x);
  void  setValue(float x);
  int   getId();
  int   getRegister();
  float getValue();

  // -- simply returns the value stored in fTECData
  float getTECRegister(int itec, std::string regname);
  int   getTECRegisterIdx(std::string rname);

  // -- read from and write to CAN
  float getTECRegisterFromCAN(int canID, std::string regname);
  void  setTECRegister(int itec, std::string regname, float value);

  // -- fill 'all' parameters for dumping into csv or for refreshing the GUI
  void readAllParamsFromCAN();

  // -- AFTER readCANmessage() these can be used to get the relevant value
  float getCANReadFloatVal() {return fCANReadFloatVal;}
  int   getCANReadIntVal() {return fCANReadIntVal;}


  void  printToGUI(std::string);
  void  getMessage(std::string);

public slots:
  void  sentFromServer(const QString&);

signals:
  void  signalSomething(int x);
  void  signalText(QString x);
  void  sendToServer(const QString&);
  void  startServer();

protected:
  void        run() override;
  void        initTECData();
  TECData     initAllTECRegister();

private:
  tLog&   fLOG;
  QMutex fMutex;
  QWaitCondition fCondition;

  QThread   *fRpcThread;
//rpc  rpcServer *fRpcServer;

  bool    fRestart;
  bool    fAbort;
  int     fCANId;
  int     fCANReg;
  float   fCANVal;
  float   fTECParameter;
  int     fValveMask; 
  QString fDateAndTime;

  int     fCANReadIntVal;
  float   fCANReadFloatVal;

  std::string fCsvFileName;
  std::ofstream fCsvFile;


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
