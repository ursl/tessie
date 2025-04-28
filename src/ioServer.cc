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
ioServer::ioServer(string name) : fName(name) {
  // should not be here?!
  //  fCtrlTessie = new tMosq("tessie", "ctrlTessie", "localhost", 1883);

}


// ----------------------------------------------------------------------
ioServer::~ioServer() {
  delete fCtrlTessie;
}


// ----------------------------------------------------------------------
void ioServer::sentToServer(QString msg) {
  if (0) cout << "ioServer::sentToServer received " << msg.toStdString() << endl;
  while (1) {
    bool ok = fCtrlTessie->sendMessage(msg.toStdString().c_str());
    if (ok) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}


// ----------------------------------------------------------------------
void ioServer::sentToMonitor(QString msg) {
  if (0) cout << "ioServer::sentToMonitor received " << msg.toStdString() << endl;
  while (1) {
    bool ok = fMoniTessie->sendMessage(msg.toStdString().c_str());
    if (ok) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}


// ----------------------------------------------------------------------
void ioServer::doRun() {
  cout << "ioServer::doRun() entered, instantiate tMosq" <<endl;
  fCtrlTessie = new tMosq(fName.c_str(), fName.c_str(), "localhost", 1883);
  fMoniTessie = new tMosq("tessieMoni", "monTessie", "localhost", 1883);

  startServer();

  while (1) {
    // -- allow signals to reach slots
    QCoreApplication::processEvents();

    //cout << "ioServer::doRun() while(1) loop, nmsg = " << nmsg << endl;
    if (fCtrlTessie->getNMessages() > 0) {
      string msg = fCtrlTessie->getMessage();
      if (0) cout << "ioServer::doRun> Qt emit sendFromServer("
                  << msg
                  << ")" << endl;
      QString qmsg = QString::fromStdString(msg);
      emit signalSendFromServer(qmsg);
// -- monTessie is write only!
//    } else if (fMoniTessie->getNMessages() > 0) {
//      string msg = fMoniTessie->getMessage();
//      cout << "ioServer::doRun> Qt emit sendFromServer("
//           << msg
//           << ")" << endl;
//      QString qmsg = QString::fromStdString(msg);
//      emit signalSendFromServer(qmsg);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
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
    int rm = fMoniTessie->loop();
    if (rm) {
      fMoniTessie->reconnect();
    } else {
      break;
    }
  }
  //run();
}


