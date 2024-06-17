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

  fFont2.setFamilies({QString::fromUtf8("Menlo")});
  fFont2.setPointSize(14);

  QWidget *fWdg = new QWidget(this);
  fWdg->setFocusPolicy(Qt::NoFocus);
  // -- Top vertical stack of layouts
  QVBoxLayout *vlayTop = new QVBoxLayout(fWdg);

  // -- top row, containing (left) info block and (right) env block
  QHBoxLayout *hlay0 = new QHBoxLayout(fWdg);
  vlayTop->addLayout(hlay0);

  // -- Info block
  QGridLayout *glay00 = new QGridLayout();
  fWdg->setLayout(glay00);

  QLabel *lblA = new QLabel("CANbus errors"); setupLBL(lblA);
  QLabel *lblB = new QLabel("I2C errors"); setupLBL(lblB);
  QLabel *lblC = new QLabel("Runtime"); setupLBL(lblC);
  QLabel *lblD = new QLabel("Version"); setupLBL(lblD);

  fqleCANbusErrors = new QLineEdit(fWdg); setupQLE(fqleCANbusErrors);
  fqleI2CErrors    = new QLineEdit(fWdg); setupQLE(fqleI2CErrors);
  fqleRunTime      = new QLineEdit(fWdg); setupQLE(fqleRunTime);
  fqleVersion      = new QLineEdit(fWdg); setupQLE(fqleVersion); fqleVersion->setFont(fFont2);

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
  fWdg->setLayout(glay01);

  QLabel *lblA1 = new QLabel("Air Temp."); setupLBL(lblA1);
  QLabel *lblB1 = new QLabel("Water Temp."); setupLBL(lblB1);
  QLabel *lblC1 = new QLabel("Rel. Hum.");  setupLBL(lblC1);
  QLabel *lblD1 = new QLabel("Dew Point");  setupLBL(lblD1);

  fqleAT = new QLineEdit(fWdg); setupQLE(fqleAT); fqleAT->setFixedSize(QSize(80, 50));
  fqleWT = new QLineEdit(fWdg); setupQLE(fqleWT); fqleWT->setFixedSize(QSize(80, 50));
  fqleRH = new QLineEdit(fWdg); setupQLE(fqleRH); fqleRH->setFixedSize(QSize(80, 50));
  fqleDP = new QLineEdit(fWdg); setupQLE(fqleDP); fqleDP->setFixedSize(QSize(80, 50));

  glay01->addWidget(lblA1,  0, 0, 1, 1);
  glay01->addWidget(fqleAT, 0, 1, 1, 1);
  glay01->addWidget(lblB1,  1, 0, 1, 1);
  glay01->addWidget(fqleWT, 1, 1, 1, 1);
  glay01->addWidget(lblC1,  0, 2, 1, 1);
  glay01->addWidget(fqleRH, 0, 3, 1, 1);
  glay01->addWidget(lblD1,  1, 2, 1, 1);
  glay01->addWidget(fqleDP, 1, 3, 1, 1);

  hlay0->addLayout(glay01);


  // -- bottom row, containing (left) buttons and (right) TEC array
  QHBoxLayout *hlay1 = new QHBoxLayout(fWdg);
  vlayTop->addLayout(hlay1);

  // -- buttons
  const QSize btnSize = QSize(100, 50);
  QVBoxLayout *vlay00 = new QVBoxLayout(fWdg);

  fbtnValve0 = new QPushButton("Flush");
  fbtnValve0->setFocusPolicy(Qt::NoFocus);
  fbtnValve0->setFont(fFont1);
  fbtnValve0->setFixedSize(btnSize);
  fbtnValve0->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  connect(fbtnValve0, &QPushButton::clicked, this, &MainWindow::btnValve0);
  vlay00->addWidget(fbtnValve0);

  fbtnValve1 = new QPushButton("Rinse");
  fbtnValve1->setFocusPolicy(Qt::NoFocus);
  fbtnValve1->setFont(fFont1);
  fbtnValve1->setFixedSize(btnSize);
  fbtnValve1->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  connect(fbtnValve1, &QPushButton::clicked, this, &MainWindow::btnValve1);
  vlay00->addWidget(fbtnValve1);

  QPushButton *btn3 = new QPushButton("Quit"); btn3->setFocusPolicy(Qt::NoFocus);
  btn3->setFont(fFont1);
  btn3->setFixedSize(btnSize);
  connect(btn3, &QPushButton::clicked, this, &MainWindow::btnQuit);
  vlay00->addWidget(btn3);

  hlay1->addLayout(vlay00);


  // -- TEC block
  QGridLayout *glay02 = new QGridLayout();
  fWdg->setLayout(glay02);

  for (unsigned int i = 0; i < 8; ++i) {
    mkTEC(i);
    fqleTEC[i]->setFixedSize(QSize(80, 50));
  }

  for (unsigned int i = 0; i < 8; ++i) {
    // int iy = (i < 4 ?0 :1);
    // int ix = 2 * (i%4);
    int iy = (i < 4 ?1 :0);
    int ix = (i < 4 ?2*(i%4) : 2*((7-i)%4));
    cout << "TEC idx=" << i << " ix = " << ix << " iy = " << iy << endl;
    glay02->addWidget(flblTEC[i], iy, ix,   1, 1);
    glay02->addWidget(fqleTEC[i], iy, ix+1, 1, 1);
  }

  hlay1->addLayout(glay02);

  setCentralWidget(fWdg);

  fqleCANbusErrors->clearFocus();

  int trsp(200);
  QPalette p0; p0.setColor(QPalette::Base, QColor(  0,   0, 250, trsp)); fPalettes.push_back(p0);
  QPalette p1; p1.setColor(QPalette::Base, QColor( 20,  80, 250, trsp)); fPalettes.push_back(p1);
  QPalette p2; p2.setColor(QPalette::Base, QColor( 20, 250, 250, trsp)); fPalettes.push_back(p2);
  QPalette p3; p3.setColor(QPalette::Base, QColor( 20, 240, 180, trsp)); fPalettes.push_back(p3);
  QPalette p4; p4.setColor(QPalette::Base, QColor( 10, 200,  10, trsp)); fPalettes.push_back(p4);
  QPalette p5; p5.setColor(QPalette::Base, QColor( 80, 240,   0, trsp)); fPalettes.push_back(p5);
  QPalette p6; p6.setColor(QPalette::Base, QColor(240, 240,   0, trsp)); fPalettes.push_back(p6);
  QPalette p7; p7.setColor(QPalette::Base, QColor(240,  80,   0, trsp)); fPalettes.push_back(p7);
  QPalette p8; p8.setColor(QPalette::Base, QColor(240,   0,   0, trsp)); fPalettes.push_back(p8);

}


// ----------------------------------------------------------------------
MainWindow::~MainWindow() {
}

// ----------------------------------------------------------------------
void MainWindow::updateHardwareDisplay() {
  fqleRunTime->setText(QString::number(fpHw->getRunTime()));

  double temp = fpHw->getTemperature();
  fqleAT->setText(QString::number(temp, 'f', 2));
  fqleAT->setPalette(fPalettes[colorReducedIndex(temp)]);

  temp = fpHw->getTECRegister(8, "Temp_W");
  fqleWT->setText(QString::number(temp, 'f', 2));
  fqleWT->setPalette(fPalettes[colorReducedIndex(temp)]);

  fqleRH->setText(QString::number(fpHw->getRH(), 'f', 2));

  fqleDP->setText(QString::number(fpHw->getDP(), 'f', 2));
  if (fabs(fpHw->getDP() - fpHw->getTemperature()) < 2.)  {
    fqleDP->setPalette(fPalettes[8]);
  }


  fqleCANbusErrors->setText(QString::number(fpHw->getNCANbusErrors()));
  if (fpHw->redCANErrors() > 0) {
    cout << "Setting CANbus error counter line edit to red" << endl;
    fqleCANbusErrors->setStyleSheet("QLineEdit {background-color : red; }");
    //    system("/usr/bin/cvlc --play-and-exit houstonwehaveaproblem_loud.mp3 &");
  } else {
    fqleCANbusErrors->setStyleSheet("QLineEdit {background-color : white; }");
  }

  fqleI2CErrors->setText(QString::number(fpHw->getNI2CErrors()));
  if (fpHw->redI2CErrors() > 0) {
    cout << "Setting I2C error counter line edit to red" << endl;
    fqleI2CErrors->setStyleSheet("QLineEdit {background-color : red; }");
    //    system("/usr/bin/cvlc --play-and-exit houstonwehaveaproblem_loud.mp3  &");
  } else {
    fqleI2CErrors->setStyleSheet("QLineEdit {background-color : white; }");
  }

  if (fpHw->getStatusValve0()) {
    fbtnValve0->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    fbtnValve0->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }
  if (fpHw->getStatusValve1()) {
    fbtnValve1->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    fbtnValve1->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }

  for (unsigned int i = 0; i < fqleTEC.size(); ++i) {
    double temp = fpHw->getTECRegister(i+1, "Temp_M");
    fqleTEC[i]->setText(QString::number(temp, 'f', 2));
    fqleTEC[i]->setPalette(fPalettes[colorIndex(temp)]);
  }

}

// ----------------------------------------------------------------------
void MainWindow::setupQLE(QLineEdit *qle) {
  qle->setFocusPolicy(Qt::NoFocus);
  qle->setAlignment(Qt::AlignRight);
  qle->setAlignment(Qt::AlignRight);
  qle->setFont(fFont1);
  qle->setFixedSize(QSize(150, 50));
}


// ----------------------------------------------------------------------
void MainWindow::setupLBL(QLabel *q) {
  q->setFocusPolicy(Qt::NoFocus);
  q->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  q->setFont(fFont1);
}


// ----------------------------------------------------------------------
void MainWindow::btnQuit() {
  static int cnt(0);
  cout << "MainWindow::signalQuitProgram(), cnt = " << cnt << endl;
  ++cnt;
  emit signalQuitProgram();
}


// ----------------------------------------------------------------------
void MainWindow::btnValve0() {
  // -- negate!
  if (!fpHw->getStatusValve0()) {
    fbtnValve0->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    fbtnValve0->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }

  emit signalValve(1);
}


// ----------------------------------------------------------------------
void MainWindow::btnValve1() {
  // -- negate!
  if (!fpHw->getStatusValve1()) {
    fbtnValve1->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    fbtnValve1->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }
  emit signalValve(2);
}


// ----------------------------------------------------------------------
void MainWindow::mkTEC(int i) {
  QString slbl = "TEC " + QString::number(i+1);
  cout << "slbl = " << slbl.toStdString() << endl;
  QLabel *lbl = new QLabel(slbl);
  setupLBL(lbl);
  lbl->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
  flblTEC.push_back(lbl);

  QLineEdit *qle = new QLineEdit();
  setupQLE(qle);
  qle->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
  fqleTEC.push_back(qle);
}


// ----------------------------------------------------------------------
int MainWindow::colorIndex(double temp)  {
  if (temp < -30.) return 0;
  if (temp < -20.) return 1;
  if (temp < -10.) return 2;
  if (temp < 0.)   return 3;
  if (temp < 10.)  return 4;
  if (temp < 20.)  return 5;
  if (temp < 30.)  return 6;
  if (temp < 40.)  return 7;
  if (temp > 40.)  return 8;
  return 0;

}

// ----------------------------------------------------------------------
int MainWindow::colorReducedIndex(double temp)  {
  if (temp < 10.)  return 0;
  if (temp < 20.)  return 1;
  if (temp < 30.)  return 4;
  if (temp > 40.)  return 8;
  return 0;

}
