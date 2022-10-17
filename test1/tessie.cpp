#include "MainWindow.h"
#include "tLog.hh"

#include <QApplication>

// -------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  tLog LOG;

  QApplication a(argc, argv);
  MainWindow w(LOG, nullptr);
  LOG.setGui(&w);
  std::cout << "w.show() call" << std::endl;
  w.show();
  std::cout << "w.show() done" << std::endl;
  return a.exec();
}
