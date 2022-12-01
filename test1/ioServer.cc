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
  cout << "rpcServer::sentToServer() received ->" << result.toStdString() << "<-" << endl;
 // emit("ioServer result emitted");
}


// ----------------------------------------------------------------------
void ioServer::run() {
  cout << "ioServer::run() starting RPC server" << endl;
  startServer();
  cout << "ioServer::run() after starting RPC server" << endl;
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
  cout << "connected. " << endl;

}


// ----------------------------------------------------------------------
void ioServer::printFromServer(const QString &result) {
  emit sendFromServer(result);
}

