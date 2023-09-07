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
            fMapTecMode.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecModeSet(ix)));
            break;
          case 2:
            fMapTecControlVoltageSet.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecVoltageSet()));
            break;
          case 3:
            fMapTecPIDkp.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDkpSet()));
            break;
          case 4:
            fMapTecPIDki.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDkiSet()));
            break;
          case 5:
            fMapTecPIDkd.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDkdSet()));
            break;
          case 6:
            fMapTecPIDMax.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDMaxSet()));
            break;
          case 7:
            fMapTecPIDMin.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDMinSet()));
            break;
          case 8:
            fMapTecRefU.insert(make_pair(ix, pte));
            QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecRefUSet()));
            break;

          default:
            break;
        }
        pte->setMaximumHeight(height);
        pte->setText(QString::number(fThread->getTECRegister(ix, regs[iy].toStdString())));
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



// -------------------------------------------------------------------------------
void TECExpert::tecVoltageSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "ControlVoltage_Set", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecModeSet(int itec) {
  for(auto it: fMapTecMode) {
    if (itec != it.first) continue;
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "Mode", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecPIDkpSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "PID_kp", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecPIDkiSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "PID_ki", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecPIDkdSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "PID_kd", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecPIDMaxSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "PID_Max", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecPIDMinSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "PID_Min", xval);
  }
}


// -------------------------------------------------------------------------------
void TECExpert::tecRefUSet() {
  for(auto it: fMapTecControlVoltageSet) {
    QString sval = it.second->text();
    float xval = sval.toFloat();
    cout << "xval = " << xval << " it.first = " << it.first << endl;
    fThread->setTECRegister(it.first, "Ref_U", xval);
  }
}
