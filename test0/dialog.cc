#include "dialog.hh"
#include "ui_dialog.h"

dialog::dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog) {
  ui->setupUi(this);
}

dialog::~dialog() {
  delete ui;
}
