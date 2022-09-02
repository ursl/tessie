#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <qpushbutton.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget * wdg = new QWidget(this);
    QPushButton *train_button = new QPushButton(wdg);
    train_button->setText(tr("something"));
    setCentralWidget(wdg);
}

MainWindow::~MainWindow()
{
    delete ui;
}

