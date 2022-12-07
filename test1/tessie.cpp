#include <QApplication>
#include <QtCore/QObject>

#include "MainWindow.h"
#include "tLog.hh"
#include "driveHardware.hh"
#include "ioServer.hh"


// -------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  tLog LOG;

  QThread *hwThread = new QThread();
  driveHardware *hw = new driveHardware(LOG);
  hw->moveToThread(hwThread);

  QThread *ioThread = new QThread();
  ioServer *io = new ioServer();
  io->moveToThread(ioThread);

  QApplication a(argc, argv);
  std::cout << "MainWindow w() call" << std::endl;
  MainWindow w(LOG, hw, nullptr);

  // -- ioServer signals
  QObject::connect(ioThread, SIGNAL(started()), io, SLOT(run()));
  bool success = QObject::connect(io, SIGNAL(signalSendFromServer(QString)), hw, SLOT(sentFromServer(QString)));
  Q_ASSERT(success);
  success = QObject::connect(hw, SIGNAL(signalSendToServer(QString)), io, SLOT(sentToServer(QString)));
  Q_ASSERT(success);

  // -- driveHardware signals
  QObject::connect(hwThread, SIGNAL(started()), hw, SLOT(run()));

  // -- MainWindow slots and signals
  QObject::connect(hw, SIGNAL(signalUpdateHwDisplay()), &w, SLOT(updateHardwareDisplay()));
  QObject::connect(hw, SIGNAL(signalText(QString)), &w, SLOT(appendText(QString)));

  QObject::connect(&w, SIGNAL(signalValve(int)), hw, SLOT(toggleFras(int)));
  QObject::connect(&w, SIGNAL(signalTurnOnTEC(int)), hw, SLOT(turnOnTEC(int)));
  QObject::connect(&w, SIGNAL(signalTurnOffTEC(int)), hw, SLOT(turnOffTEC(int)));

  // -- stuff
  QObject::connect(&LOG, SIGNAL(signalText(QString)), &w, SLOT(appendText(QString)));

  ioThread->start();
  hwThread->start();

  // -- this must be after setGui(...)!
  LOG(INFO, "start tessie");

  std::cout << "MainWindow w.show() call" << std::endl;
  w.show();
  std::cout << "MainWindow w.show() done" << std::endl;
  return a.exec();
}
