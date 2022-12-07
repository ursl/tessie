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


  /*
QThread* thread = new QThread;
Worker* worker = new Worker();
worker->moveToThread(thread);
connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
connect(thread, SIGNAL(started()), worker, SLOT(process()));
connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
thread->start();
  */


  QThread *hwThread = new QThread();
  driveHardware *hw = new driveHardware(LOG);


  QThread *ioThread = new QThread();
  ioServer *io = new ioServer();
  QObject::connect(io, SIGNAL(sendFromServer(std::string)), hw, SLOT(sentFromServer(std::string)));


  io->moveToThread(ioThread);
  ioThread->start();

  std::cout << "LOG.setGUI(&w) call" << std::endl;
  LOG.setGui(&w);
  // -- this must be after setGui(...)!
  LOG(INFO, "start tessie");

  std::cout << "MainWindow w.show() call" << std::endl;
  w.show();
  std::cout << "MainWindow w.show() done" << std::endl;
  return a.exec();
}
