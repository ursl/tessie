#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <qpushbutton.h>

// -------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
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

