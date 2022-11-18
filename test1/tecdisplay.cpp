#include "TECDisplay.h"
#include "ui_TECDisplay.h"

TECDisplay::TECDisplay(QWidget *parent):  QWidget(parent), ui(new Ui::TECDisplay) {
  ui->setupUi(this);
}

TECDisplay::~TECDisplay() {
  delete ui;
}
