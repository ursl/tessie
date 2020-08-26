#include <iostream>
#include <sstream>

#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtWidgets/QScrollBar>

#include <QtCore/QDateTime>
#include <QtCharts/QDateTimeAxis>


#include "gui.hh"

using namespace std;
using namespace QtCharts;

gui::gui(tLog &x, QMainWindow *parent): QMainWindow(parent), fLOG(x), fThread(x) {

  fLOG.setHw(&fThread);

  // -- connect the ui_gui (QML design) with the GUI class:
  ui = new Ui::MainWindow();
  ui->setupUi(this);
  updateTime();

  // -- set up a timer to display a clock
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &gui::updateTime);
  timer->start(1000);

  string welcome = fLOG.print(ALL, " *** Welcome to Tessie's *** ");


  ui->textEdit->setText(QString::fromStdString(welcome));
  ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());

  this->show();


  // -- set up display chart
  fSeries = new QLineSeries();

  QDateTime momentInTime;
  momentInTime.setDate(QDate(1991, 06, 15));
  fSeries->append(momentInTime.toMSecsSinceEpoch(), 123.);

  momentInTime.setDate(QDate(1993, 06, 15));
  fSeries->append(momentInTime.toMSecsSinceEpoch(), 139.);

  fChart = new QChart();

  fChart->addSeries(fSeries);
  fChart->legend()->hide();
  fChart->setTitle("CPU load");

  ui->graphicsView->setChart(fChart);
  ui->graphicsView->setRenderHint(QPainter::Antialiasing);

  connect(&fThread, &driveHardware::signalText, this, &gui::appendText);
  connect(&fLOG, &tLog::signalText, this, &gui::appendText);

}

gui::~gui() {
}

// ----------------------------------------------------------------------
void gui::printText(std::string line) {
  QString qline = QString::fromStdString(line);
  appendText(qline);
}


void gui::updateTime() {
  ui->label_3->setText(getTimeString());
}

void gui::appendText(QString line) {
  ui->textEdit->append(line);
}


void gui::on_pushButton_clicked() {
  fLOG(INFO, "Start");
  fThread.runPrintout(1,1);
}

void gui::on_pushButton_2_clicked() {
  fLOG(INFO, "Exit button clicked");
#ifdef PI
  fThread.shutDown();
#endif

  exit(0);
}

void gui::on_spinBox_valueChanged(int arg1) {
  stringstream sbla; sbla << "spinBox_valueChanged to " << arg1;
  fLOG(ALL, sbla.str());
  fThread.setFrequency(arg1);
}

void gui::on_spinBox_2_valueChanged(int arg1) {
  fThread.setOffset(arg1);
}

QString gui::getTimeString() {
  QDateTime dati = QDateTime::currentDateTime();
  int seconds = dati.time().second();
  int minutes = dati.time().minute();
  int hours   = dati.time().hour();
  int year    = dati.date().year();
  int month   = dati.date().month();
  int day     = dati.date().day();
  QString text;
  text = QString("%1/%2/%3 %4:%5:%6")
    .arg(year,4)
    .arg(month,2,10,QChar('0'))
    .arg(day,2,10,QChar('0'))
    .arg(hours,2,10,QChar('0'))
    .arg(minutes,2,10,QChar('0'))
    .arg(seconds,2,10,QChar('0'));
  return text;
}

#ifdef PI
void gui::on_toolButton_clicked(bool checked) {
  fLOG(INFO, "Toggle blue LED clicked");
  fThread.toggleBlue();
}
#endif
