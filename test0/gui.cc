#include <iostream>

#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtWidgets/QScrollBar>

#include "gui.hh"

gui::gui(QMainWindow *parent): QMainWindow(parent) {

    // -- here you connect the ui_gui (QML design) with the GUI class:
    // Ui::MainWindow ui;
    ui = new Ui::MainWindow();
    ui->setupUi(this);
    updateTime();

    // -- set up a timer to display a clock
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &gui::updateTime);
    timer->start(1000);

    ui->textEdit->setText("Welcome to Tessie's");
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());

    this->show();

    connect(&fThread, &driveHardware::signalText, this, &gui::appendText);

}

gui::~gui() {
}



void gui::updateTime() {
    ui->label_3->setText(getTimeString());
}

void gui::appendText(QString line) {
    ui->textEdit->append(line);
}


void gui::on_pushButton_clicked() {
    fThread.runPrintout(1,1);
}

void gui::on_pushButton_2_clicked() {
    exit(0);
}

void gui::on_spinBox_valueChanged(int arg1) {
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
    fThread.toggleBlue();
}
#endif
