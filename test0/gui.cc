#include <iostream>

#include <QtGui/QPainter>

#include "gui.hh"

gui::gui(QMainWindow *parent): QMainWindow(parent) {

    // -- here you connect the ui_gui (QML design) with the GUI class:
    Ui::MainWindow ui;
    ui.setupUi(this);
    this->show();

    connect(&fThread, &driveHardware::signalSomething,
            this, &gui::updateFrequency);

}

gui::~gui() {
}


void gui::updateFrequency(int f) {
    std::cout << "do something with f = " << f << std::endl;
}

void gui::paintEvent(QPaintEvent * /* event */) {
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
