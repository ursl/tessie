#include "ioServer.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <chrono>
#include <thread>

#include "driveHardware.hh"

using namespace std;

// ----------------------------------------------------------------------
ioServer::ioServer(driveHardware *h): fHardware(h) {
  fCtrlTessie = new tMosq("tessie", "ctrlTessie", "localhost", 1883);

//  connect(fHardware, &driveHardware::sendToServer, this, &ioServer::sentToServer);
}


// ----------------------------------------------------------------------
ioServer::~ioServer() {
  delete fCtrlTessie;
}


// ----------------------------------------------------------------------
void ioServer::sentToServer(string msg) {
  while (1) {
    bool ok = fCtrlTessie->sendMessage(msg.c_str());
    usleep(50000);
    if (ok) break;
    //    if (fCtrlTessie->published()) break;
  }
}


// ----------------------------------------------------------------------
void ioServer::run() {
  chrono::milliseconds milli5 = chrono::milliseconds(5);
  int cntMsg(0);
  while (1) {
    int nmsg = fCtrlTessie->getNMessages();
    if (nmsg != cntMsg) {
      string msg = fCtrlTessie->getMessage();
      cout << "ioServer: ->" << msg << "<-" << endl;
      cntMsg = nmsg;
      printFromServer(msg);

    } else {
      std::this_thread::sleep_for(milli5);
    }
  }
}


// ----------------------------------------------------------------------
void ioServer::startServer() {
  cout << "ioServer::startServer()" << endl;
  while(1) {
    int rc = fCtrlTessie->loop();
    if (rc) {
      fCtrlTessie->reconnect();
    } else {
      break;
    }
  }
  run();
}


// ----------------------------------------------------------------------
void ioServer::printFromServer(string msg) {
  QString qmsg = QString::fromStdString(msg);
  //fHardware->getIoMessage(msg);
}
