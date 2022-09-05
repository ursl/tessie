#include "MainWindow.h"
#include "tLog.hh"

#include <QApplication>

// -------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  tLog LOG;

  QApplication a(argc, argv);
    MainWindow w(LOG, nullptr);
    LOG.setGui(&w);
    w.show();
    return a.exec();
}
