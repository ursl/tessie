#include "MainWindow.hh"

#include <sstream>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <QtWidgets>
#include <qpushbutton.h>
#include <unistd.h>

#include "tLog.hh"
#include "sha256.hh"
#include "util.hh"

using namespace std;

// -------------------------------------------------------------------------------
MainWindow::MainWindow(tLog &x, driveHardware *h, QWidget *parent) {
  QMainWindow *mw = new QMainWindow(parent);
  mw->setEnabled(true);
  mw->resize(814, 564);


  QWidget *wdg = new QWidget(this);
  QVBoxLayout *vlay = new QVBoxLayout(wdg);
  QPushButton *btn1 = new QPushButton("");
  vlay->addWidget(btn1);
  QPushButton *btn2 = new QPushButton("btn2");
  vlay->addWidget(btn2);
  QPushButton *btn3 = new QPushButton("Quit");
  connect(btn3, &QPushButton::clicked, this, &MainWindow::btnQuit);

  vlay->addWidget(btn3);
  wdg->setLayout(vlay);
  setCentralWidget(wdg);
}


// ----------------------------------------------------------------------
MainWindow::~MainWindow() {
}

// ----------------------------------------------------------------------
void MainWindow::updateHardwareDisplay() {
  cout << "MainWindow::updateHardwareDisplay()" << endl;
}

// ----------------------------------------------------------------------
void MainWindow::btnQuit() {
  static int cnt(0);
  cout << "MainWindow::signalQuitProgram(), cnt = " << cnt << endl;
  ++cnt;
  emit signalQuitProgram();
}
