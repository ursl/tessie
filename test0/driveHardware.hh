#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTime>

#include "tLog.hh"

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
  void toggleBlue();
  void shutDown();
#endif

  void setFrequency(int x);
  void setOffset(int x);
  int  getFrequency();
  int  getOffset();

  void  printToGUI(std::string);

signals:
  void signalSomething(int x);
  void signalText(QString x);

protected:
  void run() override;

private:
  tLog&   fLOG;
  QMutex fMutex;
  QWaitCondition fCondition;

  bool    fRestart;
  bool    fAbort;
  int     fFrequency;
  int     fOffset;
  QString fDateAndTime;

#ifdef PI
  // -- Red LED: Physical pin 18, BCM GPIO24, and WiringPi pin 5.
  const int fLed1 = 5;
  // -- Red LED: Physical pin 22, BCM GPIO25, and WiringPi pin 6.
  const int fLedBlue = 6;

  int fStatus1 = 0;
  int fStatusBlue = 0;

#endif

};

#endif // DRIVEHARDWARE_H
