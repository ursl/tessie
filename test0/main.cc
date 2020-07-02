#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cstdio>
#include <thread>

#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>

#include <QtCore/QThread>

#include "dialog.hh"
#include "driveHardware.hh"


int startGui(int, char **);
int run_driveHardware(int, int);

using namespace std;


// ----------------------------------------------------------------------
int main(int argc, char **argv) {

  int bla1 = startGui(argc, argv);

  return 0;
}


// ----------------------------------------------------------------------
int startGui(int argc, char **argv) {
  //  cout << "startGui: " << hnote << endl;
  QApplication app(argc, argv);

  dialog *gui = new dialog();
  gui->show();

  QThread *thread = QThread::create([]{run_driveHardware(1234, 12); });
  thread->start();


  return app.exec();
}


// ----------------------------------------------------------------------
int run_driveHardware(int startVal, int freq) {
  driveHardware a(freq, startVal);
  a.runPrintout();

  return 0;
}
