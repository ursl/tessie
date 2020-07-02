#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>

#include <chrono>
#include <thread>

using namespace std;

// ----------------------------------------------------------------------
driveHardware::driveHardware(QObject *parent): QThread(parent) {
  fRestart   = false;
  fAbort     = false;
  fFrequency = 0;
  fOffset    = 0;
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {
  fMutex.lock();
  fAbort = true;
  fCondition.wakeOne();
  fMutex.unlock();

  wait();
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
    int cnt = this->fOffset;
    std::chrono::milliseconds sec(1000/this->fFrequency);
    cout << "countUp: " << cnt++ << " fFrequency = " << fFrequency << endl;
    //    sleep(1./fFrequency);
    fMutex.unlock();
    std::this_thread::sleep_for(sec);

    // -- I think I don't need the following:
    // fMutex.lock();
    // if (!fRestart) {
    //   fCondition.wait(&fMutex);
    // }
    // fRestart = false;
    // fMutex.unlock();
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
