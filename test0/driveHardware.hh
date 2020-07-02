#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

class driveHardware: public QThread {
  Q_OBJECT

public:
  driveHardware(QObject *parent = nullptr);
  ~driveHardware();

  void runPrintout(int freq, int off);

  void setFrequency(int x);
  void setOffset(int x);
  int  getFrequency();
  int  getOffset();

signals:
  void signalSomething(int x);

protected:
  void run() override;

private:
  QMutex fMutex;
  QWaitCondition fCondition;

  bool fRestart;
  bool fAbort;
  int fFrequency;
  int fOffset;
};

#endif // DRIVEHARDWARE_H
