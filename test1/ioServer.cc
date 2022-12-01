#include "ioServer.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <chrono>
#include <thread>

#include "driveHardware.hh"


using namespace std;

// ----------------------------------------------------------------------
ioServer::ioServer(driveHardware *h) {
  fMosq = new tMosq("tessie", "ctrlTessie", "localhost", 1883);
}


// ----------------------------------------------------------------------
ioServer::~ioServer() {
  delete fMosq;
}


// ----------------------------------------------------------------------
void ioServer::sentToServer(const QString &result) {
  cout << "ioServer::sentToServer() received ->" << result.toStdString() << "<-" << endl;
 // emit("ioServer result emitted");
}


// ----------------------------------------------------------------------
void ioServer::run() {
  chrono::milliseconds milli5 = chrono::milliseconds(5);
  int cntMsg(0);
  while (1) {
    int nmsg = fMosq->getNMessages();
    if (nmsg != cntMsg) {
      string msg = fMosq->getMessage();
      cout << "ioServer: ->" << msg << "<-" << endl;
      cntMsg = nmsg;
    } else {
      std::this_thread::sleep_for(milli5);
    }
  }
}


// ----------------------------------------------------------------------
void ioServer::startServer() {
  cout << "ioServer::startServer()" << endl;
  while(1) {
    int rc = fMosq->loop();
    if (rc) {
      fMosq->reconnect();
    } else {
      break;
    }
  }
  run();
}


// ----------------------------------------------------------------------
void ioServer::printFromServer(const QString &result) {
  emit sendFromServer(result);
}

