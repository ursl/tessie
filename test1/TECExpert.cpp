#include "TECExpert.h"

#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>

#include <sstream>

using namespace std;

// -------------------------------------------------------------------------------
TECExpert::TECExpert(QWidget *, driveHardware *x) : fThread(x) {
  fUI = new QWidget();
  int height(35);
  fUI->setFixedSize(QSize(500, (1+5)*height));

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

  QLineEdit *pte;
  for (int ix = 1; ix <= 8; ++ix) {
      for (int iy = 1; iy <= 5; ++iy) {
        pte = new QLineEdit(fUI);
        pte->setMaximumHeight(height);
        switch (iy) {
        case 1:
            pte->setText(QString::number(fThread->getTECRegister(ix, "Mode")));
            break;
        case 2:
            pte->setText(QString::number(fThread->getTECRegister(ix, "ControlVoltage_Set")));
            break;
        case 3:
            pte->setText(QString::number(fThread->getTECRegister(ix, "PID_kp")));
            break;
        case 4:
            pte->setText(QString::number(fThread->getTECRegister(ix, "PID_ki")));
            break;
        case 5:
            pte->setText(QString::number(fThread->getTECRegister(ix, "PID_kd")));
            break;
        default:
            break;
        }
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
