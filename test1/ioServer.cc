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
  // cout << "rpcServer::rpcServer() starting RPC server" << endl;
  // startServer();
}


// ----------------------------------------------------------------------
ioServer::~ioServer() {

}


// ----------------------------------------------------------------------
void ioServer::sentToServer(const QString &result) {
  cout << "rpcServer::sentToServer() received ->" << result.toStdString() << "<-" << endl;
  emit("ioServer result emitted");
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
}


// ----------------------------------------------------------------------
void ioServer::printFromServer(const QString &result) {
  emit sendFromServer(result);
}

