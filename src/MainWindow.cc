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

  const QSize btnSize = QSize(100, 50);

  this->resize(800, 480);
  //  this->setStyleSheet("background-color: rgba(211, 211, 211, 60%);");

  // -- FIXME: Test whether fWdg can be replaced by this
  QWidget *fWdg = new QWidget(this);
  fWdg->setFocusPolicy(Qt::NoFocus);

  // -- Top vertical stack of layouts
  QVBoxLayout *vlayTop = new QVBoxLayout(fWdg);

  // -- header row
  QHBoxLayout *hlay = new QHBoxLayout(/*fWdg*/);
  vlayTop->addLayout(hlay);
  ifstream INS;
  string sline;
  INS.open("version.txt");
  getline(INS, sline);
  INS.close();

  QLabel *lbl = new QLabel(string("TESSIE  (" + sline + ") " + fpHw->getHostname()).c_str()); setupLBL(lbl);
  lbl->setStyleSheet("font-weight: bold;");

  hlay->addWidget(lbl);

  fLOG(INFO, "tessie version " + sline);

  QPushButton *btn1 = new QPushButton("Test\nsound"); btn1->setFocusPolicy(Qt::NoFocus);
  btn1->setFont(fFont1);
  btn1->setStyleSheet("QPushButton {background-color: rgba(211, 211, 211, 0.4);color: black;}");
  btn1->setFixedSize(QSize(70, 50));
  connect(btn1, &QPushButton::clicked, this, &MainWindow::btnSound);
  hlay->addWidget(btn1);

  QPushButton *btn2 = new QPushButton("Interlock reset"); btn2->setFocusPolicy(Qt::NoFocus);
  btn2->setFont(fFont1);
  btn2->setStyleSheet("QPushButton {background-color: rgba(154, 100, 100, 0.4);color: black; font-weight: bold;}");
  btn2->setFixedSize(QSize(200, 50));
  connect(btn2, &QPushButton::clicked, this, &MainWindow::btnINTL);
  hlay->addWidget(btn2);

  QPushButton *btn3 = new QPushButton("Quit"); btn3->setFocusPolicy(Qt::NoFocus);
  btn3->setFont(fFont1);
  btn3->setStyleSheet("QPushButton {color: black; font-weight: bold;}");
  btn3->setFixedSize(QSize(80, 50));
  connect(btn3, &QPushButton::clicked, this, &MainWindow::btnQuit);
  hlay->addWidget(btn3);

  QSpacerItem *spacer0 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  vlayTop->addSpacerItem(spacer0);

  // -- top row, containing (left) info block and (right) env block
  QHBoxLayout *hlay0 = new QHBoxLayout(/*fWdg*/);
  vlayTop->addLayout(hlay0);

  // -- Info block
  QGridLayout *glay00 = new QGridLayout();
  glay00->setAlignment(Qt::AlignTop);

  //  fWdg->setLayout(glay00);

  QLabel *lblA = new QLabel("Errors (CANbus/I2C)"); setupLBL(lblA);
  QLabel *lblB = new QLabel("Flow switch"); setupLBL(lblB);
  QLabel *lblE = new QLabel("Free diskspace"); setupLBL(lblE);
  QLabel *lblC = new QLabel("Runtime"); setupLBL(lblC);
  QLabel *lblD = new QLabel("Status"); setupLBL(lblD);

  fqleBusErrors = new QLineEdit(/*fWdg*/); setupQLE(fqleBusErrors);
  fqleFlowSwitch   = new QLineEdit(/*fWdg*/); setupQLE(fqleFlowSwitch);
  fqleFreeDisk     = new QLineEdit(/*fWdg*/); setupQLE(fqleFreeDisk);
  fqleRunTime      = new QLineEdit(/*fWdg*/); setupQLE(fqleRunTime);
  fqleStatus       = new QLineEdit(/*fWdg*/); setupQLE(fqleStatus); fqleStatus->setFont(fFont2);

  glay00->addWidget(lblA,  0, 0, 1, 1);
  glay00->addWidget(fqleBusErrors, 0, 1, 1, 1);

  glay00->addWidget(lblE,  1, 0, 1, 1);
  glay00->addWidget(fqleFreeDisk, 1, 1, 1, 1);

  glay00->addWidget(lblB,  2, 0, 1, 1);
  glay00->addWidget(fqleFlowSwitch, 2, 1, 1, 1);

  glay00->addWidget(lblC,  3, 0, 1, 1);
  glay00->addWidget(fqleRunTime, 3, 1, 1, 1);

  glay00->addWidget(lblD,  4, 0, 1, 1);
  glay00->addWidget(fqleStatus, 4, 1, 1, 1);

  hlay0->addLayout(glay00);

  QSpacerItem *spacerx = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  hlay0->addSpacerItem(spacerx);

  // -- Environmental block
  QGridLayout *glay01 = new QGridLayout();
  glay01->setAlignment(Qt::AlignTop);
  //  fWdg->setLayout(glay01);

  QLabel *lblA1 = new QLabel("Air [\xC2\xB0 C]"); setupLBL(lblA1);
  QLabel *lblB1 = new QLabel("Water  [\xC2\xB0 C]"); setupLBL(lblB1);
  QLabel *lblC1 = new QLabel("Rel. Hum.");  setupLBL(lblC1);
  QLabel *lblD1 = new QLabel("Dew Point");  setupLBL(lblD1);

  QLabel *lblE1 = new QLabel("Lid status");  setupLBL(lblE1);
  QLabel *lblF1 = new QLabel("Interlock");   setupLBL(lblF1);

  fqleAT = new QLineEdit(fWdg); setupQLE(fqleAT); fqleAT->setFixedSize(QSize(80, 50));
  fqleWT = new QLineEdit(fWdg); setupQLE(fqleWT); fqleWT->setFixedSize(QSize(80, 50));
  fqleRH = new QLineEdit(fWdg); setupQLE(fqleRH); fqleRH->setFixedSize(QSize(80, 50));
  fqleDP = new QLineEdit(fWdg); setupQLE(fqleDP); fqleDP->setFixedSize(QSize(80, 50));
    
  fqleLS = new QLineEdit(fWdg); setupQLE(fqleLS); fqleLS->setFixedSize(QSize(80, 50));
  fqleIL = new QLineEdit(fWdg); setupQLE(fqleIL); fqleIL->setFixedSize(QSize(80, 50));

  glay01->addWidget(lblA1,  0, 0, 1, 1);
  glay01->addWidget(fqleAT, 0, 1, 1, 1);
  glay01->addWidget(lblB1,  1, 0, 1, 1);
  glay01->addWidget(fqleWT, 1, 1, 1, 1);
  glay01->addWidget(lblE1,  2, 0, 1, 1);
  glay01->addWidget(fqleLS, 2, 1, 1, 1);

  glay01->addWidget(lblC1,  0, 2, 1, 1);
  glay01->addWidget(fqleRH, 0, 3, 1, 1);
  glay01->addWidget(lblD1,  1, 2, 1, 1);
  glay01->addWidget(fqleDP, 1, 3, 1, 1);
  glay01->addWidget(lblF1,  2, 2, 1, 1);
  glay01->addWidget(fqleIL, 2, 3, 1, 1);


  QPushButton *btn4 = new QPushButton("STOP ALL"); btn4->setFocusPolicy(Qt::NoFocus);
  btn4->setFont(fFont2);
  btn4->setFixedSize(QSize(150, 50));
  btn4->setStyleSheet("QPushButton {background-color: rgba(204, 50, 50, 0.4); color: black; font-weight: bold; border: 3px solid}");
  btn4->update();
  connect(btn4, &QPushButton::clicked, this, &MainWindow::btnStop);
  glay01->addWidget(btn4, 3, 0, 1, 2, Qt::AlignLeft);

  QPushButton *btn5 = new QPushButton("Restart tessieWeb"); btn5->setFocusPolicy(Qt::NoFocus);
  btn5->setFont(fFont2);
  btn5->setFixedSize(QSize(220, 50));
  btn5->setStyleSheet("QPushButton {background-color: rgba(120, 120, 120, 0.4); color: black; font-weight: bold; border: 3px solid}");
  btn5->update();
  connect(btn5, &QPushButton::clicked, this, &MainWindow::btnRestartTessieWeb);
  //no  glay01->minimumWidth = 10;
  glay01->addWidget(btn5, 3, 2, 1, 2, Qt::AlignRight);

  hlay0->addLayout(glay01);

  QSpacerItem *spacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  vlayTop->addSpacerItem(spacer1);

  // -- bottom row, containing (left) buttons and (right) TEC array
  QHBoxLayout *hlay1 = new QHBoxLayout(/*fWdg*/);
  vlayTop->addLayout(hlay1);

  // -- buttons
  QVBoxLayout *vlay00 = new QVBoxLayout(/*fWdg*/);

  fStyleSheet.insert(make_pair("valveOffThrottleOff", "QPushButton {background-color: gray; font-weight: bold; color: black}"));
  fStyleSheet.insert(make_pair("valveOffThrottleOn", "QPushButton {background-color: gray; font-weight: bold; color: white}"));
  fStyleSheet.insert(make_pair("valveOnThrottleOn", "QPushButton {background-color: #46923c; font-weight: bold; color: white}"));
  fStyleSheet.insert(make_pair("valveOnThrottleOff", "QPushButton {background-color: #46923c; font-weight: bold; color: black}"));
  
  fbtnValve0 = new QPushButton("Flush");
  fbtnValve0->setFocusPolicy(Qt::NoFocus);
  fbtnValve0->setFont(fFont1);
  fbtnValve0->setFixedSize(btnSize);
  fbtnValve0->setStyleSheet(fStyleSheet["valveOffThrottleOff"]);
  connect(fbtnValve0, &QPushButton::clicked, this, &MainWindow::btnValve0);
  vlay00->addWidget(fbtnValve0);

  fbtnValve1 = new QPushButton("Rinse");
  fbtnValve1->setFocusPolicy(Qt::NoFocus);
  fbtnValve1->setFont(fFont1);
  fbtnValve1->setFixedSize(btnSize);
  fbtnValve1->setStyleSheet(fStyleSheet["valveOffThrottleOff"]);
  connect(fbtnValve1, &QPushButton::clicked, this, &MainWindow::btnValve1);
  vlay00->addWidget(fbtnValve1);

  hlay1->addLayout(vlay00);


  // -- TEC block
  QGridLayout *glay02 = new QGridLayout();
  //  fWdg->setLayout(glay02);

  for (unsigned int i = 0; i < 8; ++i) {
    mkTEC(i);
    fqleTEC[i]->setFixedSize(QSize(80, 50));
  }

  for (unsigned int i = 0; i < 8; ++i) {
    // int iy = (i < 4 ?0 :1);
    // int ix = 2 * (i%4);
    int iy = (i < 4 ?1 :0);
    int ix = (i < 4 ?2*(i%4) : 2*((7-i)%4));
    //    cout << "TEC idx=" << i << " ix = " << ix << " iy = " << iy << endl;
    glay02->addWidget(flblTEC[i], iy, ix,   1, 1);
    glay02->addWidget(fqleTEC[i], iy, ix+1, 1, 1);
  }

  hlay1->addLayout(glay02);

  //  QSpacerItem *spacer2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  //  vlayTop->addSpacerItem(spacer2);

  
  setCentralWidget(fWdg);

  fqleBusErrors->clearFocus();

  int trsp(200);

  // dark blue #0000fa
  QPalette p0; p0.setColor(QPalette::Base, QColor(  0,   0, 250, trsp)); fPalettes.push_back(p0);
  // blue #1450fa
  QPalette p1; p1.setColor(QPalette::Base, QColor( 20,  80, 250, trsp)); fPalettes.push_back(p1);
  // cyan #14fafa
  QPalette p2; p2.setColor(QPalette::Base, QColor( 20, 250, 250, trsp)); fPalettes.push_back(p2);
  // green-blue #14f0b4
  QPalette p3; p3.setColor(QPalette::Base, QColor( 20, 240, 180, trsp)); fPalettes.push_back(p3);
  // darker green #0ac80a
  QPalette p4; p4.setColor(QPalette::Base, QColor( 10, 200,  10, trsp)); fPalettes.push_back(p4);
  // light green #50f000
  QPalette p5; p5.setColor(QPalette::Base, QColor( 80, 240,   0, trsp)); fPalettes.push_back(p5);
  // yellow #f0f000
  QPalette p6; p6.setColor(QPalette::Base, QColor(240, 240,   0, trsp)); fPalettes.push_back(p6);
  // orange #f05000
  QPalette p7; p7.setColor(QPalette::Base, QColor(240,  80,   0, trsp)); fPalettes.push_back(p7);
  // red #f00000
  QPalette p8; p8.setColor(QPalette::Base, QColor(240,   0,   0, trsp)); fPalettes.push_back(p8);
  // white #ffffff
  QPalette p9; p9.setColor(QPalette::Base, QColor(255, 255, 255, trsp)); fPalettes.push_back(p9);

}


// ----------------------------------------------------------------------
MainWindow::~MainWindow() {
}

// ----------------------------------------------------------------------
void MainWindow::updateHardwareDisplay() {
  static bool isred(false);
  static int cnt(0);
  
  fqleRunTime->setText(QString::number(fpHw->getRunTime()));

  string ss = fpHw->getStatusString();
  fqleStatus->setText(ss.c_str());
  if (string::npos != ss.find("chiller")) {
    fqleStatus->setPalette(fPalettes[8]);
  } else  if (string::npos != ss.find("no problem")) {
    fqleStatus->setPalette(fPalettes[4]);
  } else  if (string::npos != ss.find("emergency")) {
    fqleStatus->setPalette(fPalettes[8]);
  } else {
    fqleStatus->setPalette(fPalettes[6]);
  }
  
  double temp = fpHw->getTemperature();
  fqleAT->setText(QString::number(temp, 'f', 2));
  fqleAT->setPalette(fPalettes[colorReducedIndex(temp)]);

  temp = fpHw->getTECRegister(8, "Temp_W");
  fqleWT->setText(QString::number(temp, 'f', 2));
  fqleWT->setPalette(fPalettes[colorReducedIndex(temp)]);

  double rh = fpHw->getRH();
  fqleRH->setText(QString::number(rh, 'f', 2));
  if (rh < 5.) {
    fqleRH->setPalette(fPalettes[3]);
  } else  {
    fqleRH->setPalette(fPalettes[4]);
  }

  double dp = fpHw->getDP();
  fqleDP->setText(QString::number(fpHw->getDP(), 'f', 2));
  if ((temp - dp) < 2.)  {
    fqleDP->setPalette(fPalettes[8]);
  } else {
    fqleDP->setPalette(fPalettes[4]);
  }
  
  fqleBusErrors->setText(QString::number(fpHw->getNCANbusErrors()) + "/" + QString::number(fpHw->getNI2CErrors()));
  if ((fpHw->redCANErrors() > 0) || (fpHw->redI2CErrors() > 0)) {
    fqleBusErrors->setPalette(fPalettes[8]);
  } else {
    fqleBusErrors->setPalette(fPalettes[9]);
  }

  fqleFlowSwitch->setText(QString::number(fpHw->getFlowSwitchStatus()));
  if (fpHw->getFlowSwitchStatus() < 1) {
    fqleFlowSwitch->setPalette(fPalettes[8]);

  } else {
    fqleFlowSwitch->setPalette(fPalettes[4]);
  }

  if (0 == cnt) {
    int freedisk = fpHw->getFreeDisk(); 
    fqleFreeDisk->setText(QString::number(freedisk));
    if (freedisk > 2) {
      fqleFreeDisk->setPalette(fPalettes[4]);
    } else if ((freedisk >= 1) && (freedisk < 2)) {
      fqleFreeDisk->setPalette(fPalettes[6]);
    } else {
      fqleFreeDisk->setPalette(fPalettes[8]);
    }

  } else {
    // -- reset once per hour
    if (3600 == cnt) cnt = -1;
  }
  ++cnt;

if (fpHw->getStatusValve0()) {
  if (fpHw->getThrottleStatus()) {
  fbtnValve0->setStyleSheet(fStyleSheet["valveOnThrottleOn"]);
    } else {
      fbtnValve0->setStyleSheet(fStyleSheet["valveOnThrottleOff"]);
    }
  } else {
    if (fpHw->getThrottleStatus()) {
      fbtnValve0->setStyleSheet(fStyleSheet["valveOffThrottleOn"]);
    } else {
      fbtnValve0->setStyleSheet(fStyleSheet["valveOffThrottleOff"]);
    }
  }

  if (fpHw->getStatusValve1()) {
    if (fpHw->getThrottleStatus()) {
      fbtnValve1->setStyleSheet(fStyleSheet["valveOnThrottleOn"]);
    } else {
      fbtnValve1->setStyleSheet(fStyleSheet["valveOnThrottleOff"]);
    }
  } else {
    if (fpHw->getThrottleStatus()) {
      fbtnValve1->setStyleSheet(fStyleSheet["valveOffThrottleOn"]);
    } else {
      fbtnValve1->setStyleSheet(fStyleSheet["valveOffThrottleOff"]);
    }
  }

  int ls = fpHw->getLidStatus();
  if (ls == 1) {
    fqleLS->setText("locked");
    fqleLS->setPalette(fPalettes[4]);
  } else if (ls == 0)  {
    fqleLS->setText("unlocked");
    fqleLS->setPalette(fPalettes[6]);
  } else if (ls == -1)  {
    fqleLS->setText("open");
    fqleLS->setPalette(fPalettes[6]);
  } else  {
    fqleLS->setText("????");
    fqleLS->setPalette(fPalettes[8]);
  }

  if (fpHw->getInterlockStatus()) {
    fqleIL->setText("good");
    fqleIL->setPalette(fPalettes[4]);
  } else {
    fqleIL->setText("bad");
    fqleIL->setPalette(fPalettes[8]);
  }

  if ((temp - dp) < 2.)  {
    fqleDP->setPalette(fPalettes[8]);
  } else {
    fqleDP->setPalette(fPalettes[4]);
  }


  for (unsigned int i = 0; i < fqleTEC.size(); ++i) {
    double temp = fpHw->getTECRegister(i+1, "Temp_M");
    fqleTEC[i]->setText(QString::number(temp, 'f', 2));
    fqleTEC[i]->setPalette(fPalettes[colorIndex(temp)]);
    if ((temp - dp) < 2) {
      if (!isred) fqleTEC[i]->setPalette(fPalettes[8]);
    }

    if (fpHw->getTECRegister(i+1, "PowerState")) {
      flblTEC[i]->setStyleSheet("font-weight: bold; background-color: #A3C1DA");
    } else {
      flblTEC[i]->setStyleSheet("font-weight: normal; background-color: rgba(211, 211, 211, 60%)");
    }

  }

  isred = !isred;

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
void MainWindow::btnINTL() {
  fLOG(INFO, "MainWindow::btnINTL() clicked");
  fpHw->resetInterlock();
}


// ----------------------------------------------------------------------
void MainWindow::btnStop() {
  static int cnt(0);
  cout << "MainWindow::signalStopProgram(99), cnt = " << cnt << endl;
  ++cnt;
#ifdef UZH
  emit signalValve(4);
#else
  emit signalStopOperations(99);
#endif
}

// ----------------------------------------------------------------------
void MainWindow::btnRestartTessieWeb() {
  cout << "MainWindow::btnRestartTessieWeb" << endl;
  system("/usr/bin/sudo systemctl restart tessieWeb");
}


// ----------------------------------------------------------------------
void MainWindow::btnValve0() {
  // -- negate!
  if (!fpHw->getStatusValve0()) {
    fbtnValve0->setStyleSheet("QPushButton {background-color: #46923c; color: black; font-weight: bold;}");
  } else {
    fbtnValve0->setStyleSheet("QPushButton {background-color: gray; color: black; font-weight: bold;}");
  }

  emit signalValve(1);
}


// ----------------------------------------------------------------------
void MainWindow::btnValve1() {
  // -- negate!
  if (!fpHw->getStatusValve1()) {
    fbtnValve1->setStyleSheet("QPushButton {background-color: #46923c; color: black; font-weight: bold;}");
  } else {
    fbtnValve1->setStyleSheet("QPushButton {background-color: gray; color: black; font-weight: bold;}");
  }
  emit signalValve(2);
}


// ----------------------------------------------------------------------
void MainWindow::mkTEC(int i) {
  QString slbl = "TEC " + QString::number(i+1);
  //  cout << "slbl = " << slbl.toStdString() << endl;
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
  if (temp < -30.) return 0; // dark blue
  if (temp < -20.) return 1; // blue
  if (temp < -10.) return 2; // cyan
  if (temp < 0.)   return 3; // green-blue
  if (temp < 10.)  return 4; // darker green
  if (temp < 20.)  return 5; // light green
  if (temp < 25.)  return 6; // yellow
  if (temp < 30.)  return 7; // orange
  if (temp > 30.)  return 8; // red
  return 0;

}

// ----------------------------------------------------------------------
int MainWindow::colorReducedIndex(double temp)  {
  if (temp < 10.)  return 1;
  if (temp < 20.)  return 2;
  if (temp < 25.)  return 4;
  if (temp > 25.)  return 8;
  return 0;

}

// ----------------------------------------------------------------------
void MainWindow::showAlarm(int state) {
  static int cnt(50);
  if (0 == state) {
    // -- reset everything when alarm condition is gone
    killSiren();
    fAlarmSoundPlaying = false;
    cnt = 50;
    return;
  } else if (1 == state) {
    // -- this is the normal alarm setup
    fLOG(INFO, "showAlarm! cnt = " + std::to_string(cnt));
    if (!fAlarmSoundPlaying) {
      system("/usr/bin/cvlc -R ../siren.mp3 &");
      fAlarmSoundPlaying = true;
    }
    --cnt;
    // -- after 50 invocations, reset counter and allow a fresh start of siren playing
    if (cnt < 1) {
      killSiren();
      fAlarmSoundPlaying = false;
      cnt = 50;
    }
  }
}


// ----------------------------------------------------------------------
void MainWindow::btnSound() {
  fLOG(INFO, "MainWindow::btnSound() clicked");
  system("/usr/bin/cvlc ../siren.mp3 & ");
  //  system("/usr/bin/cvlc -R ../19seconds.mp3 ");
}


// ----------------------------------------------------------------------
void MainWindow::killSiren() {
  cout << "MainWindow::killSiren() entered" << endl;
  char bfr[1023] ;
  FILE *fp;
  if ((fp = popen("ps -ef | grep vlc","r")) == NULL) {
    cout << "something bad happened" << endl;
    // error processing and return
  }
 
  while(fgets(bfr, BUFSIZ, fp) != NULL){
    string sbfr(bfr);
    sbfr.pop_back();
    cleanupString(sbfr);
    vector<string> tokens = split(sbfr, ' ');
    cout << "->" << sbfr << "<-" << " PID = " << tokens[1] << endl;
    string scommand = "/usr/bin/kill -9 " + tokens[1];
    fLOG(INFO, scommand);
    system(scommand.c_str());
    fAlarmSoundPlaying = false;
  }
  pclose(fp);
 
}
