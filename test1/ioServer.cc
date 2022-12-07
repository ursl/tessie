#include "ioServer.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <QCoreApplication>

#include <chrono>
#include <thread>
#include <sys/ioctl.h>

using namespace std;

// ----------------------------------------------------------------------
ioServer::ioServer() {
  // should not be here?!
  //  fCtrlTessie = new tMosq("tessie", "ctrlTessie", "localhost", 1883);

}


// ----------------------------------------------------------------------
ioServer::~ioServer() {
  delete fCtrlTessie;
}


// ----------------------------------------------------------------------
void ioServer::sentToServer(QString msg) {
  cout << "ioServer received " << msg.toStdString() << endl;
  while (1) {
    bool ok = fCtrlTessie->sendMessage(msg.toStdString().c_str());
    usleep(50000);
    if (ok) break;
    //    if (fCtrlTessie->published()) break;
  }
}


// ----------------------------------------------------------------------
void ioServer::run() {
  cout << "ioServer::run() entered, instantiate tMosq" <<endl;
  fCtrlTessie = new tMosq("tessie", "ctrlTessie", "localhost", 1883);

  cout << "startServer()" << endl;
  startServer();
  cout << "  .. done" << endl;

  int cntMsg(0);
 // while (1) {
    // -- allow signals to reach slots
    QCoreApplication::processEvents();

    int nmsg = fCtrlTessie->getNMessages();
    //cout << "ioServer::run() while(1) loop, nmsg = " << nmsg << endl;
    if (nmsg != cntMsg) {
      string msg = fCtrlTessie->getMessage();
      cout << "ioServer: ->" << msg << "<-" << endl;
      cntMsg = nmsg;
      cout << "emit sendFromServer("
           << msg
           << ")" << endl;
      QString qmsg = QString::fromStdString(msg);
      emit signalSendFromServer(qmsg);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
 // }
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
  //run();
}


