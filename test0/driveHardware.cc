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

  fRpcThread = new QThread();
  fRpcServer = new rpcServer(this);
  //  connect(fRpcThread, &QThread::finished, fRpcServer, &QObject::deleteLater);
  connect(this, &driveHardware::sendToServer, fRpcServer, &rpcServer::sentToServer);
  connect(this, &driveHardware::startServer, fRpcServer, &rpcServer::run);
  connect(fRpcServer, &rpcServer::sendFromServer, this, &driveHardware::sentFromServer);
  fRpcServer->moveToThread(fRpcThread);
  fRpcThread->start();
  emit startServer();
  //  fRpcServer->run();

#ifdef PI

#endif
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {

  fMutex.lock();
  fAbort = true;
  fCondition.wakeOne();
  fMutex.unlock();

  fRpcThread->quit();
  fRpcThread->wait();

  wait();
#ifdef PI
  shutDown();
#endif
}

// ----------------------------------------------------------------------
void driveHardware::sentFromServer(const QString &result) {
  cout << "driveHardware::sentFromServer() -> " << result.toStdString() << "<-" << endl;
  signalText(result);
}


// ----------------------------------------------------------------------
void  driveHardware::getMessage(std::string x) {
  QString aline(QString::fromStdString(x));
  signalText(aline);

  // -- do something else eventually
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
void driveHardware::sendCANmessage(unsigned int id, unsigned int reg, char[4] bytes) {
}

char[4] driveHardware::readCANmessage(unsigned int id, unsigned int reg) {
  char[4] result = {0, 0, 0, 0};
  return result;
}
#endif
