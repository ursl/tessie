#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTime>

#include "tLog.hh"
#include "rpcServer.hh"

#ifdef PI
#include <wiringPi.h>
#endif

class driveHardware: public QThread {
  Q_OBJECT

public:
  driveHardware(tLog &x, QObject *parent = nullptr);
  ~driveHardware();

  void runPrintout(int freq, int off);

#ifdef PI
  void    sendCANmessage(unsigned int id, unsigned int reg, char[4] bytes);
  char[4] readCANmessage(unsigned int id, unsigned int reg);
#endif

  void setFrequency(int x);
  void setOffset(int x);
  int  getFrequency();
  int  getOffset();

  void  printToGUI(std::string);
  void  getMessage(std::string);

public slots:
  void sentFromServer(const QString&);

signals:
  void signalSomething(int x);
  void signalText(QString x);
  void sendToServer(const QString&);
  void startServer();

protected:
  void run() override;

private:
  tLog&   fLOG;
  QMutex fMutex;
  QWaitCondition fCondition;

  QThread   *fRpcThread;
  rpcServer *fRpcServer;

  bool    fRestart;
  bool    fAbort;
  int     fFrequency;
  int     fOffset;
  QString fDateAndTime;

#ifdef PI

#endif

};

#endif // DRIVEHARDWARE_H
