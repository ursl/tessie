#include <QApplication>
#include <QtCore/QObject>

#include "MainWindow.hh"
#include "tLog.hh"
#include "driveHardware.hh"
#include "ioServer.hh"


using namespace std;

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
