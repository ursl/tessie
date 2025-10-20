#include <QApplication>
#include <QtCore>

#include <chrono>
#include <thread>

#include "MainWindow.hh"
#include "tLog.hh"
#include "driveHardware.hh"
#include "ioServer.hh"


using namespace std;

void quitProgram();


// -------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  tLog LOG;

  // -- by default run in full screen mode
  bool doRunFullScreen(false);
  int verbose(0);

  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-h")) {
      cout << "List of arguments:" << endl;
      cout << "-d [DEBUG|INFO|WARNING|ERROR|ALL] set debug level" << endl;
      cout << "-f                                start in fullscreen (not window)" << endl;
      return 0;
    }
    if (!strcmp(argv[i], "-f")) {doRunFullScreen = true;}
    if (!strcmp(argv[i], "-d")) {LOG.setLevel(argv[++i]);}
    if (!strcmp(argv[i], "-v")) {verbose = 1;}
  }


  QThread *hwThread = new QThread();
  driveHardware *hw = new driveHardware(LOG, verbose);
  hw->moveToThread(hwThread);

  QThread *ioThread = new QThread();
  ioServer *io = new ioServer();
  io->moveToThread(ioThread);

  QApplication a(argc, argv);
  std::cout << "MainWindow w() call" << std::endl;
  MainWindow w(LOG, hw, nullptr);


  // -- ioServer signals
  QObject::connect(ioThread, SIGNAL(started()), io, SLOT(doRun()));
  bool success = QObject::connect(io, SIGNAL(signalSendFromServer(QString)), hw, SLOT(sentFromServer(QString)));
  Q_ASSERT(success);
  success = QObject::connect(hw, SIGNAL(signalSendToServer(QString)), io, SLOT(sentToServer(QString)));
  Q_ASSERT(success);
  success = QObject::connect(hw, SIGNAL(signalSendToMonitor(QString)), io, SLOT(sentToMonitor(QString)));
  Q_ASSERT(success);

  // -- driveHardware signals
  QObject::connect(hwThread, SIGNAL(started()), hw, SLOT(doRun()));

  // -- MainWindow slots and signals
  QObject::connect(hw, SIGNAL(signalUpdateHwDisplay()), &w, SLOT(updateHardwareDisplay()));
  QObject::connect(hw, SIGNAL(signalAlarm(int)), &w, SLOT(showAlarm(int)));

  QObject::connect(&w, &MainWindow::signalQuitProgram, quitProgram);
  QObject::connect(&w, SIGNAL(signalValve(int)), hw, SLOT(toggleFras(int)));
  QObject::connect(&w, SIGNAL(signalStopOperations(int)), hw, SLOT(stopOperations(int)));
  QObject::connect(&w, &MainWindow::btnStartReconditioning, hw, &driveHardware::doReconditioning);
  
  ioThread->start();
  hwThread->start();

  // -- this must be after setGui(...)!
  LOG(INFO, "start tessie");

  if (doRunFullScreen) {
    std::cout << "MainWindow w.showFullScreen() call" << std::endl;
    w.showFullScreen();
  } else {
    std::cout << "MainWindow w.show() call" << std::endl;
    w.show();
  }

  std::cout << "MainWindow w.show() done" << std::endl;
  return a.exec();
}


// ----------------------------------------------------------------------
void quitProgram() {
  cout << "tessie> quitProgram()" << endl;
  exit(0);
}
