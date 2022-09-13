#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTime>

#include "tLog.hh"
//rpc #include "rpcServer.hh"

#ifdef PI
//#include <wiringPi.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

class driveHardware: public QThread {
  Q_OBJECT

public:
  driveHardware(tLog &x, QObject *parent = nullptr);
  ~driveHardware();

  void  runPrintout(int reg, float val);

  void  sendCANmessage();
  void  readCANmessage();

  // -- controlling the FRAS/valve(s)
  void  talkToFras();
  void  toggleFras(int imask);
  void  entertainFras();
  void  shutDown();

  // -- controlling the TEC
  void  setTECParameter(float par);
  void  turnOnTEC(int itec);
  void  turnOffTEC(int itec);

  //
  void  setId(int x);
  void  setRegister(int x);
  void  setValue(float x);
  int   getId();
  int   getRegister();
  float getValue();

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
  void  run() override;

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

};

#endif // DRIVEHARDWARE_H
