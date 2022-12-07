#include <QApplication>
#include <QtCore/QObject>

#include "MainWindow.h"
#include "tLog.hh"
#include "driveHardware.hh"
#include "ioServer.hh"


// -------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  tLog LOG;

  QApplication a(argc, argv);
  std::cout << "MainWindow w() call" << std::endl;
  MainWindow w(LOG, nullptr);

  QThread *hwThread = new QThread();
  driveHardware *hw = new driveHardware(LOG);

  QThread *ioThread = new QThread();
  ioServer *io = new ioServer();

  // -- ioServer signals
  QObject::connect(io, SIGNAL(sendFromServer(std::string)), hw, SLOT(sentFromServer(std::string)));
  QObject::connect(ioThread, SIGNAL(started()), io, SLOT(run()));

  // -- driveHardware signals
  QObject::connect(hwThread, SIGNAL(started()), hw, SLOT(run()));

  // -- MainWindow slots and signals
  QObject::connect(hw, SIGNAL(updateHwDisplay()), &w, SLOT(updateHardwareDisplay()));
  QObject::connect(hw, SIGNAL(signalText(std::string)), &w, SLOT(appendText(std::string)));

  QObject::connect(&w, SIGNAL(signalValve(int)), hw, SLOT(toggleFras(int)));
  QObject::connect(&w, SIGNAL(signalTurnOnTEC(int)), hw, SLOT(turnOnTEC(int)));
  QObject::connect(&w, SIGNAL(signalTurnOffTEC(int)), hw, SLOT(turnOffTEC(int)));

  // -- stuff
  QObject::connect(&LOG, SIGNAL(signalText(std::string)), &w, SLOT(appendText(std::string)));


  io->moveToThread(ioThread);
  ioThread->start();

  hw->moveToThread(hwThread);
  hwThread->start();

  // -- this must be after setGui(...)!
  LOG(INFO, "start tessie");

  std::cout << "MainWindow w.show() call" << std::endl;
  w.show();
  std::cout << "MainWindow w.show() done" << std::endl;
  return a.exec();
}
