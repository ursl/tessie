#include "MainWindow.h"

#include <sstream>

#include <qpushbutton.h>
#include <unistd.h>

#include "tLog.hh"
#include "ui_MainWindow.h"


using namespace std;

// -------------------------------------------------------------------------------
MainWindow::MainWindow(tLog &x, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), fLOG(x), fThread(x) {
  fLOG.setHw(&fThread);
  ui->setupUi(this);
}

// -------------------------------------------------------------------------------
MainWindow::~MainWindow() {
  delete ui;
}


// ----------------------------------------------------------------------
void MainWindow::appendText(QString line) {
//  ui->textEditLog->append(line);
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
      ui->checkBoxTEC0->setChecked(state);
      break;
    case 1:
      ui->checkBoxTEC1->setChecked(state);
      break;
    case 2:
      ui->checkBoxTEC2->setChecked(state);
      break;
    case 3:
      ui->checkBoxTEC3->setChecked(state);
      break;
    case 4:
      ui->checkBoxTEC4->setChecked(state);
      break;
    case 5:
      ui->checkBoxTEC5->setChecked(state);
      break;
    case 6:
      ui->checkBoxTEC6->setChecked(state);
      break;
    case 7:
      ui->checkBoxTEC7->setChecked(state);
      break;
  }
}


// ----------------------------------------------------------------------
void MainWindow::quitProgram() {
  stringstream sbla; sbla << "This is the end, my friend";
  ui->textEditLog->append(sbla.str().c_str());
  exit(0);
}



// ----------------------------------------------------------------------
void MainWindow::checkValve0(bool checked) {
  stringstream sbla; sbla << "checkValve0 clicked ";
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkValve1(bool checked) {
  stringstream sbla; sbla << "checkValve1 clicked ";
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkValveAll(bool checked) {
  stringstream sbla; sbla << "checkValveAll clicked ";
  ui->textEditLog->append(sbla.str().c_str());
}


// ----------------------------------------------------------------------
void MainWindow::checkTEC0(bool checked) {
  stringstream sbla; sbla << "checkTEC0 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC1(bool checked) {
  stringstream sbla; sbla << "checkTEC1 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC2(bool checked) {
  stringstream sbla; sbla << "checkTEC2 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC3(bool checked) {
  stringstream sbla; sbla << "checkTEC3 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC4(bool checked) {
  stringstream sbla; sbla << "checkTEC4 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC5(bool checked) {
  stringstream sbla; sbla << "checkTEC5 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC6(bool checked) {
  stringstream sbla; sbla << "checkTEC6 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
}

// ----------------------------------------------------------------------
void MainWindow::checkTEC7(bool checked) {
  stringstream sbla; sbla << "checkTEC7 clicked " << checked;
  ui->textEditLog->append(sbla.str().c_str());
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



