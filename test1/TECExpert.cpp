#include "TECExpert.h"

#include "MainWindow.h"

#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QObject>

#include <sstream>
#include <iostream>

using namespace std;

// -------------------------------------------------------------------------------
TECExpert::TECExpert(MainWindow *m, driveHardware *x) : fThread(x), fUI(0), fMW(m) {
  fUI = new QWidget();

  int height(35);
  map<int, QString> regs{{1,"Mode"},
                         {2,"ControlVoltage_Set"},
                         {3,"PID_kp"},
                         {4,"PID_ki"},
                         {5,"PID_kd"},
                         {6,"PID_Max"},
                         {7,"PID_Min"},
                         {8,"Ref_U"}
                         };

  fUI->setFixedSize(QSize(600, regs.size()*height));

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

  for (auto it: regs) {
      b = new QLabel(fUI);
      b->setText(it.second);
      leftLayout->addWidget(b, it.first, 0);
  }

  leftLayout->addWidget(quitButton, 0, 0);

  QLineEdit *pte;
  for (int ix = 1; ix <= 8; ++ix) {
      for (unsigned int iy = 1; iy <= regs.size(); ++iy) {
        pte = new QLineEdit(fUI);
        switch (iy) {
          case 1:
            fMapTecMode.insert(make_pair(1, pte));
            break;
          default:
            break;
        }
        pte->setMaximumHeight(height);
        pte->setText(QString::number(fThread->getTECRegister(ix, regs[iy].toStdString())));
        leftLayout->addWidget(pte, iy, ix);
        QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecVoltSet()));
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


// -------------------------------------------------------------------------------
void TECExpert::tecVoltSet() {
  for(auto it: fMapTecMode) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "ControlVoltage_Set", xval);
  }
}
