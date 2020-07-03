#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTime>

#ifdef PI
#include <wiringPi.h>
// -- Red LED: Physical pin 18, BCM GPIO24, and WiringPi pin 5.
const int gled = 5;
#endif




class driveHardware: public QThread {
  Q_OBJECT

public:
  driveHardware(QObject *parent = nullptr);
  ~driveHardware();

  void runPrintout(int freq, int off);

#ifdef PI
  void toggleLED();
#endif

  void setFrequency(int x);
  void setOffset(int x);
  int  getFrequency();
  int  getOffset();

signals:
  void signalSomething(int x);
  void signalText(QString x);

protected:
  void run() override;

private:
  QMutex fMutex;
  QWaitCondition fCondition;

  bool fRestart;
  bool fAbort;
  int fFrequency;
  int fOffset;
  QString fDateAndTime;
};

#endif // DRIVEHARDWARE_H
