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
    QDateTime dt = QDateTime::currentDateTime();
    fDateAndTime = dt.date().toString() + "  " +  dt.time().toString("hh:mm");


#ifdef PI
  wiringPiSetup();
  pinMode(gled, OUTPUT);
#endif
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
        std::chrono::milliseconds sec(1000/this->fFrequency);
        int cn = fOffset++;
        QString aline;
        aline = QString("countUp: %1 %2").arg(cn).arg(fFrequency);

        signalText(aline);

        cout << "countUp: " << cn
             << " fFrequency = " << fFrequency
             << endl;


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
void driveHardware::toggleLED() {
  static int status = 0;
  if (0 == status) {
    cout << "toggle LED on" << endl;
    status = 1;
    digitalWrite(gled, HIGH);
  } else {
    cout << "toggle LED off" << endl;
    status = 0;
    digitalWrite(gled, LOW);
  }
}
#endif
