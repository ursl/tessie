#include "MainWindow.hh"

#include <sstream>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <QtWidgets/QLineEdit>
#include <QtWidgets>

#include <qpushbutton.h>
#include <unistd.h>

#include "tLog.hh"
#include "sha256.hh"
#include "util.hh"

using namespace std;

// -------------------------------------------------------------------------------
MainWindow::MainWindow(tLog &x, driveHardware *h, QWidget *parent) :
  QMainWindow(parent), fLOG(x), fpHw(h) {

  fFont1.setFamilies({QString::fromUtf8("Menlo")});
  fFont1.setPointSize(16);

  QWidget *wdg = new QWidget(this);
  wdg->setFocusPolicy(Qt::NoFocus);

  QVBoxLayout *vlayTop = new QVBoxLayout(wdg);

  QHBoxLayout *hlay0 = new QHBoxLayout(wdg);
  vlayTop->addLayout(hlay0);

  QVBoxLayout *vlay00 = new QVBoxLayout(wdg);
  QVBoxLayout *vlay01 = new QVBoxLayout(wdg);

  // -- Info block
  QGridLayout *glay00 = new QGridLayout();
  wdg->setLayout(glay00);

  QLabel *lblA = new QLabel("CANbus errors"); setupLBL(lblA);
  QLabel *lblB = new QLabel("I2C errors"); setupLBL(lblB);
  QLabel *lblC = new QLabel("Runtime"); setupLBL(lblC);
  QLabel *lblD = new QLabel("Version"); setupLBL(lblD);

  fqleCANbusErrors = new QLineEdit(wdg); setupQLE(fqleCANbusErrors);
  fqleI2CErrors    = new QLineEdit(wdg); setupQLE(fqleI2CErrors);
  fqleRunTime      = new QLineEdit(wdg); setupQLE(fqleRunTime);
  fqleVersion      = new QLineEdit(wdg); setupQLE(fqleVersion);
  ifstream INS;
  string sline;
  INS.open("version.txt");
  getline(INS, sline);
  fqleVersion->setText(sline.c_str());
  INS.close();


  glay00->addWidget(lblA,  0, 0, 1, 1);
  glay00->addWidget(fqleCANbusErrors, 0, 1, 1, 1);
  glay00->addWidget(lblB,  1, 0, 1, 1);
  glay00->addWidget(fqleI2CErrors, 1, 1, 1, 1);
  glay00->addWidget(lblC,  2, 0, 1, 1);
  glay00->addWidget(fqleRunTime, 2, 1, 1, 1);
  glay00->addWidget(lblD,  3, 0, 1, 1);
  glay00->addWidget(fqleVersion, 3, 1, 1, 1);

  hlay0->addLayout(glay00);

  // -- Environmental block
  QGridLayout *glay01 = new QGridLayout();
  wdg->setLayout(glay01);

  QLabel *lblA1 = new QLabel("Air Temp."); setupLBL(lblA1);
  QLabel *lblB1 = new QLabel("Water Temp."); setupLBL(lblB1);
  QLabel *lblC1 = new QLabel("Rel. Hum.");  setupLBL(lblC1);
  QLabel *lblD1 = new QLabel("Dew Point");  setupLBL(lblD1);

  fqleAT = new QLineEdit(wdg); setupQLE(fqleAT);
  fqleWT = new QLineEdit(wdg); setupQLE(fqleWT);
  fqleRH = new QLineEdit(wdg); setupQLE(fqleRH);
  fqleDP = new QLineEdit(wdg); setupQLE(fqleDP);

  glay01->addWidget(lblA1,  0, 0, 1, 1);
  glay01->addWidget(fqleAT, 0, 1, 1, 1);
  glay01->addWidget(lblB1,  1, 0, 1, 1);
  glay01->addWidget(fqleWT, 1, 1, 1, 1);
  glay01->addWidget(lblC1,  0, 2, 1, 1);
  glay01->addWidget(fqleRH, 0, 3, 1, 1);
  glay01->addWidget(lblD1,  1, 2, 1, 1);
  glay01->addWidget(fqleDP, 1, 3, 1, 1);

  hlay0->addLayout(glay01);

  hlay0->addLayout(vlay00);


  QHBoxLayout *hlay1 = new QHBoxLayout(wdg);
  vlayTop->addLayout(hlay1);

  QPushButton *btn1 = new QPushButton("btn1"); btn1->setFocusPolicy(Qt::NoFocus);
  btn1->setFont(fFont1);
  vlay01->addWidget(btn1);


  QPushButton *btn3 = new QPushButton("Quit"); btn3->setFocusPolicy(Qt::NoFocus);
  btn3->setFont(fFont1);
  connect(btn3, &QPushButton::clicked, this, &MainWindow::btnQuit);
  vlay00->addWidget(btn3);

  QPushButton *btn4 = new QPushButton("Flush"); btn4->setFocusPolicy(Qt::NoFocus);
  btn4->setFont(fFont1);
  vlay00->addWidget(btn4);
  QPushButton *btn5 = new QPushButton("Rinse"); btn5->setFocusPolicy(Qt::NoFocus);
  btn5->setFont(fFont1);
  vlay00->addWidget(btn5);

  setCentralWidget(wdg);

  fqleCANbusErrors->clearFocus();

}


// ----------------------------------------------------------------------
MainWindow::~MainWindow() {
}

// ----------------------------------------------------------------------
void MainWindow::updateHardwareDisplay() {
  cout << "MainWindow::updateHardwareDisplay()" << endl;
  fqleCANbusErrors->setText(QString::number(fpHw->getNCANbusErrors()));
  fqleI2CErrors->setText(QString::number(fpHw->getNI2CErrors()));
  fqleRunTime->setText(QString::number(fpHw->getRunTime()));

  fqleAT->setText(QString::number(fpHw->getTemperature(), 'f', 2));
  fqleWT->setText(QString::number(fpHw->getTECRegister(8, "Temp_W"), 'f', 2));
  fqleRH->setText(QString::number(fpHw->getRH(), 'f', 2));
  fqleDP->setText(QString::number(fpHw->getDP(), 'f', 2));


}

// ----------------------------------------------------------------------
void MainWindow::btnQuit() {
  static int cnt(0);
  cout << "MainWindow::signalQuitProgram(), cnt = " << cnt << endl;
  ++cnt;
  emit signalQuitProgram();
}


// ----------------------------------------------------------------------
void MainWindow::setupQLE(QLineEdit *qle) {
  qle->setFocusPolicy(Qt::NoFocus);
  qle->setAlignment(Qt::AlignRight);
  qle->setFont(fFont1);
}


// ----------------------------------------------------------------------
void MainWindow::setupLBL(QLabel *q) {
  q->setFocusPolicy(Qt::NoFocus);
  q->setAlignment(Qt::AlignLeft);
  q->setFont(fFont1);
}
