#include "TECExpert.h"

#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

#include <sstream>

using namespace std;

// -------------------------------------------------------------------------------
TECExpert::TECExpert(QWidget *) {
  fUI = new QWidget();
  fUI->setFixedSize(QSize(500, 300));

  QPushButton *quitButton = new QPushButton("Close", fUI);
  connect(quitButton, &QPushButton::clicked, this, &TECExpert::close);

  QGridLayout *leftLayout = new QGridLayout;

  QLabel *b(0);
  for (int i = 1; i <= 8; ++i) {
    b = new QLabel(fUI);
    QString qTitle = QString("TEC") + QString::number(i);
    b->setText(qTitle);
    b->setFixedWidth(40);
    leftLayout->addWidget(b, 0, i);
  }

  b = new QLabel(fUI);
  b->setText("Mode");
  leftLayout->addWidget(b, 1, 0);

  b = new QLabel(fUI);
  b->setText("ControlVoltage_Set");
  leftLayout->addWidget(b, 2, 0);

  b = new QLabel(fUI);
  b->setText("PID_kp");
  leftLayout->addWidget(b, 3, 0);

  b = new QLabel(fUI);
  b->setText("PID_ki");
  leftLayout->addWidget(b, 4, 0);

  b = new QLabel(fUI);
  b->setText("PID_kd");
  leftLayout->addWidget(b, 5, 0);

  leftLayout->addWidget(quitButton, 0, 0);

  QPlainTextEdit *pte;
  for (int ix = 1; ix <= 8; ++ix) {
      for (int iy = 1; iy <= 5; ++iy) {
        pte = new QPlainTextEdit(fUI);
        pte->setMaximumHeight(40);
        pte->insertPlainText("0");
        leftLayout->addWidget(pte, iy, ix);
      }
  }
  fUI->setLayout(leftLayout);

  fUI->show();
}


// -------------------------------------------------------------------------------
TECExpert::~TECExpert() {
    delete fUI;
}


// -------------------------------------------------------------------------------
void TECExpert::close() {
    fUI->close();
    delete this;
}


// -------------------------------------------------------------------------------
void TECExpert::setHardware(driveHardware *x) {
  fThread = x;
}
