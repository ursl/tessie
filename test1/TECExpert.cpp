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
  map<int, QString> vregs{{1,"Mode"},
                         {2,"Control_Voltage"},
                         {3,"PID_kp"},
                         {4,"PID_ki"},
                         {5,"PID_kd"},
                         {6,"PID_Max"},
                         {7,"PID_Min"},
                         {8,"Ref_U"}};

  fUI->setFixedSize(QSize(600, vregs.size()*height));

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

  for (auto it: vregs) {
      b = new QLabel(fUI);
      b->setText(it.second);
      leftLayout->addWidget(b, it.first, 0);
  }

  leftLayout->addWidget(quitButton, 0, 0);

  QLineEdit *pte;
  for (int ix = 1; ix <= 8; ++ix) {
      for (int iy = 1; iy <= 8; ++iy) {
        pte = new QLineEdit(fUI);
        pte->setMaximumHeight(height);
        pte->setText(QString::number(fThread->getTECRegister(ix, vregs[iy].toStdString())));
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
