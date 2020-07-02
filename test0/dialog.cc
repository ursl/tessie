#include <iostream>

#include <QtGui/QPainter>


#include "dialog.hh"
#include "ui_dialog.h"

dialog::dialog(QWidget *parent):QWidget(parent) {

  connect(&fThread, &driveHardware::signalSomething,
	  this, &dialog::updateFrequency);
}

dialog::~dialog() {
}


void dialog::updateFrequency(int f) {
  std::cout << "do something with f = " << f << std::endl;
}

void dialog::paintEvent(QPaintEvent * /* event */) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::black);
}
