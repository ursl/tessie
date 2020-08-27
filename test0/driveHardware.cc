#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

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


#ifdef PI
  wiringPiSetup();
  pinMode(fLed1, OUTPUT);
  pinMode(fLedBlue, OUTPUT);
#endif
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {

  fMutex.lock();
  fAbort = true;
  fCondition.wakeOne();
  fMutex.unlock();

  wait();
#ifdef PI
  shutDown();
#endif
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
    if (1 == fStatus1) {
      digitalWrite(fLed1, LOW);
      fStatus1 = 0;
    } else {
      digitalWrite(fLed1, HIGH);
      fStatus1 = 1;
    }
#endif

    fMutex.unlock();
    //    sleep(1./fFrequency);
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


#ifdef PI
// ----------------------------------------------------------------------
void driveHardware::toggleBlue() {
  if (0 == fStatusBlue) {
    cout << "toggle blue LED on" << endl;
    fStatusBlue = 1;
    digitalWrite(fLedBlue, HIGH);
  } else {
    cout << "toggle LED off" << endl;
    fStatusBlue = 0;
    digitalWrite(fLedBlue, LOW);
  }
}

// ----------------------------------------------------------------------
void driveHardware::shutDown() {
  // -- turn of LEDs
  digitalWrite(fLed1, LOW);
  digitalWrite(fLedBlue, LOW);
}

#endif
