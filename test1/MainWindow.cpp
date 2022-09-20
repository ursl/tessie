#include "MainWindow.h"

#include <sstream>

#include <qpushbutton.h>
#include <unistd.h>

#include "tLog.hh"
#include "ui_MainWindow.h"


using namespace std;

// -------------------------------------------------------------------------------
MainWindow::MainWindow(tLog &x, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  fLOG(x),
  fThread(x) {
  fLOG.setHw(&fThread);
  ui->setupUi(this);

  updateHardwareValues();
}


// ----------------------------------------------------------------------
MainWindow::~MainWindow() {
  delete ui;
}


// ----------------------------------------------------------------------
void MainWindow::appendText(QString line) {
  ui->textEditLog->append(line);
}


// ----------------------------------------------------------------------
QString MainWindow::getTimeString() {
  QDateTime dati = QDateTime::currentDateTime();
  int seconds = dati.time().second();
  int minutes = dati.time().minute();
  int hours   = dati.time().hour();
  int year    = dati.date().year();
  int month   = dati.date().month();
  int day     = dati.date().day();
  QString text;
  text = QString("%1/%2/%3 %4:%5:%6")
      .arg(year, 4)
      .arg(month, 2, 10, QChar('0'))
      .arg(day, 2, 10, QChar('0'))
      .arg(hours, 2, 10, QChar('0'))
      .arg(minutes, 2, 10, QChar('0'))
      .arg(seconds, 2, 10, QChar('0'));
  return text;
}


// ----------------------------------------------------------------------
void MainWindow::setCheckBoxTEC(int itec, bool state) {
  switch (itec) {
    case 0:
      ui->checkBoxTEC1->setChecked(state);
      break;
    case 1:
      ui->checkBoxTEC2->setChecked(state);
      break;
    case 2:
      ui->checkBoxTEC3->setChecked(state);
      break;
    case 3:
      ui->checkBoxTEC4->setChecked(state);
      break;
    case 4:
      ui->checkBoxTEC5->setChecked(state);
      break;
    case 5:
      ui->checkBoxTEC6->setChecked(state);
      break;
    case 6:
      ui->checkBoxTEC7->setChecked(state);
      break;
    case 7:
      ui->checkBoxTEC8->setChecked(state);
      break;
    }
}


// ----------------------------------------------------------------------
void MainWindow::quitProgram() {
  stringstream sbla; sbla << "This is the end, my friend";
  ui->textEditLog->append(sbla.str().c_str());
  exit(0);
}


void MainWindow::clkRefresh() {
  updateHardwareValues();

}

// ----------------------------------------------------------------------
void MainWindow::start() {
  stringstream sbla; sbla << "Startup";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.runPrintout(1,1);
}


// ----------------------------------------------------------------------
void MainWindow::tec8ChangePar0() {
  ui->tec8_par0->setStyleSheet("QLineEdit {color : red; }");
  QString sval = ui->tec8_par0->text();
  float xval = sval.toFloat();
  fThread.setTECRegister(8, "ControlVoltage_Set", xval);
}

// ----------------------------------------------------------------------
void MainWindow::tec1ChangePar0() {
  ui->tec1_par0->setStyleSheet("QLineEdit {color : red; }");
  QString sval = ui->tec1_par0->text();
  float xval = sval.toFloat();
  fThread.setTECRegister(1, "ControlVoltage_Set", xval);
}

// ----------------------------------------------------------------------
void MainWindow::clkValve0() {
  stringstream sbla; sbla << "checkValve0 clicked ";
  ui->textEditLog->append(sbla.str().c_str());
  sbla << "toggleFRAS(1)";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.toggleFras(1);
}


// ----------------------------------------------------------------------
void MainWindow::clkValve1() {
  stringstream sbla; sbla << "checkValve1 clicked ";
  ui->textEditLog->append(sbla.str().c_str());
  sbla << "toggleFRAS(2)";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.toggleFras(2);
}


// ----------------------------------------------------------------------
void MainWindow::clkValveAll() {
  stringstream sbla; sbla << "checkValveAll clicked ";
  ui->textEditLog->append(sbla.str().c_str());
  sbla << "toggleFRAS(3)";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.toggleFras(3);
}



// ----------------------------------------------------------------------
void MainWindow::checkTEC0(bool checked) {
  stringstream sbla; sbla << "checkTEC0 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  ui->textEditLog->append("   do nothing!");
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC1(bool checked) {
  stringstream sbla; sbla << "checkTEC1 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(1);
    } else {
      fThread.turnOffTEC(1);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC2(bool checked) {
  stringstream sbla; sbla << "checkTEC2 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(2);
    } else {
      fThread.turnOffTEC(2);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC3(bool checked) {
  stringstream sbla; sbla << "checkTEC3 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(3);
    } else {
      fThread.turnOffTEC(3);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC4(bool checked) {
  stringstream sbla; sbla << "checkTEC4 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(4);
    } else {
      fThread.turnOffTEC(4);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC5(bool checked) {
  stringstream sbla; sbla << "checkTEC5 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(5);
    } else {
      fThread.turnOffTEC(5);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC6(bool checked) {
  stringstream sbla; sbla << "checkTEC6 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(6);
    } else {
      fThread.turnOffTEC(6);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC7(bool checked) {
  stringstream sbla; sbla << "checkTEC7 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(7);
    } else {
      fThread.turnOffTEC(7);
    }
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC8(bool checked) {
  stringstream sbla; sbla << "checkTEC8 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
  if (checked) {
      fThread.turnOnTEC(8);
    } else {
      fThread.turnOffTEC(8);
    }
}


// ----------------------------------------------------------------------
void MainWindow::checkTECAll(bool checked) {
  stringstream sbla; sbla << "checkTECTAll clicked " << checked;
  string sline = sbla.str();
  QString qline = sline.c_str();
  ui->textEditLog->append(qline);
  for (int i = 0; i < 8; ++i) {
      setCheckBoxTEC(i, checked);
    }
}


// ----------------------------------------------------------------------
void MainWindow::updateVoltageValue() {
  QString qline = ui->textTECParameter->toPlainText();
  float fbla = qline.toFloat();

  stringstream sbla; sbla << "changed TEC parameter ->" << qline.toStdString().c_str() << "<- in float: " << fbla;

  fThread.setTECParameter(fbla);
  ui->textEditLog->append(sbla.str().c_str());

}


// ----------------------------------------------------------------------
void MainWindow::updateHardwareValues() {

  QString sval;
  sval = QString::number(fThread.getTECRegister(8, "ControlVoltage_Set"), 'f', 2);
  ui->tec8_par0->setText(sval);
  ui->tec8_par0->setStyleSheet("QLineEdit {color : green; }");

  sval = QString::number(fThread.getTECRegister(7, "ControlVoltage_Set"), 'f', 2);
  ui->tec7_par0->setText(sval);
  ui->tec7_par0->setStyleSheet("QLineEdit {color : green; }");

  sval = QString::number(fThread.getTECRegister(6, "ControlVoltage_Set"), 'f', 2);
  ui->tec6_par0->setText(sval);
  ui->tec6_par0->setStyleSheet("QLineEdit {color : green; }");

  sval = QString::number(fThread.getTECRegister(5, "ControlVoltage_Set"), 'f', 2);
  ui->tec5_par0->setText(sval);
  ui->tec5_par0->setStyleSheet("QLineEdit {color : green; }");

  // FIXME

  sval = QString::number(fThread.getTECRegister(1, "ControlVoltage_Set"), 'f', 2);
  ui->tec1_par0->setText(sval);
  ui->tec1_par0->setStyleSheet("QLineEdit {color : green; }");

}
