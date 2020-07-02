#include <iostream>

#include <QtGui/QPainter>


#include "gui.hh"
#include "ui_gui.h"

gui::gui(QMainWindow *parent): QMainWindow(parent) {

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
