#include "rpcServer.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <chrono>
#include <thread>

#include "driveHardware.hh"

#include "rpc/trpc.h"

using namespace std;

// ----------------------------------------------------------------------
rpcServer::rpcServer() {
  // cout << "rpcServer::rpcServer() starting RPC server" << endl;
  // startServer();
}


// ----------------------------------------------------------------------
rpcServer::~rpcServer() {

}


// ----------------------------------------------------------------------
void rpcServer::sentToServer(const QString &result) {
  cout << "rpcServer::sendToServer() received ->" << result.toStdString() << "<-" << endl;
  emit("rpcServer result emitted");
}


// ----------------------------------------------------------------------
void rpcServer::run() {
  cout << "rpcServer::run() starting RPC server" << endl;
  startServer();

}
