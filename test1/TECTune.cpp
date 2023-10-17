#include "TECTune.h"

#include "MainWindow.h"

#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QObject>

#include <sstream>
#include <iostream>

using namespace std;

// -------------------------------------------------------------------------------
TECTune::TECTune(MainWindow *m, driveHardware *x) : fThread(x), fUI(0), fMW(m) {
  fUI = new QWidget();

  fUI->setFixedSize(QSize(1200, 600));

  QPushButton *quitButton = new QPushButton("Close", fUI);
  connect(quitButton, &QPushButton::clicked, this, &TECTune::close);

  fUI->show();
}


// -------------------------------------------------------------------------------
TECTune::~TECTune() {
    delete fUI;
}


// -------------------------------------------------------------------------------
void TECTune::close() {
    fMW->setTECModsZero();
    fUI->close();
    delete this;
}


// -------------------------------------------------------------------------------
void TECTune::setHardware(driveHardware *x) {
  fThread = x;
}


// -------------------------------------------------------------------------------
void TECTune::updateHardwareDisplay() {

}


// -------------------------------------------------------------------------------
void TECTune::tecSetFromUI(int itec, std::string rname, QLineEdit *ql) {
  QString sval = ql->text();
  float xval = sval.toFloat();
  cout << "xval = " << xval << " itec = " << itec << endl;
  fThread->setTECRegister(itec, rname, xval);
}

