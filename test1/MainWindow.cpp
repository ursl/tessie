#include "MainWindow.h"

#include <sstream>

#include <qpushbutton.h>

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


// -------------------------------------------------------------------------------
void MainWindow::on_buttonQuit_clicked() {
  exit(0);
}


void MainWindow::on_buttonValve0_clicked() {
  appendText("turn on valve0");
  stringstream sbla; sbla << "turn on valve0";
  fLOG(INFO, sbla.str());
}

// ----------------------------------------------------------------------
void MainWindow::appendText(QString line) {
//  ui->textEditLog->append(line);
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
