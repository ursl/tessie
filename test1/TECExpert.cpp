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
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1ModeSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2ModeSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3ModeSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4ModeSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5ModeSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6ModeSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7ModeSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8ModeSet()));

            break;
          case 2:
            fMapTecControlVoltageSet.insert(make_pair(ix, pte));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1VoltageSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2VoltageSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3VoltageSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4VoltageSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5VoltageSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6VoltageSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7VoltageSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8VoltageSet()));
            break;
          case 3:
            fMapTecPIDkp.insert(make_pair(ix, pte));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1PIDkpSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2PIDkpSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3PIDkpSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4PIDkpSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5PIDkpSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6PIDkpSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7PIDkpSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8PIDkpSet()));
            break;
          case 4:
            fMapTecPIDki.insert(make_pair(ix, pte));
            // QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDkiSet()));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1PIDkiSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2PIDkiSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3PIDkiSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4PIDkiSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5PIDkiSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6PIDkiSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7PIDkiSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8PIDkiSet()));
            break;
          case 5:
            fMapTecPIDkd.insert(make_pair(ix, pte));
            // QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDkdSet()));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1PIDkdSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2PIDkdSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3PIDkdSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4PIDkdSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5PIDkdSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6PIDkdSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7PIDkdSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8PIDkdSet()));
            break;
          case 6:
            fMapTecPIDMax.insert(make_pair(ix, pte));
            // QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDMaxSet()));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1PIDMaxSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2PIDMaxSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3PIDMaxSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4PIDMaxSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5PIDMaxSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6PIDMaxSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7PIDMaxSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8PIDMaxSet()));
            break;
          case 7:
            fMapTecPIDMin.insert(make_pair(ix, pte));
            //QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecPIDMinSet()));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1PIDMinSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2PIDMinSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3PIDMinSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4PIDMinSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5PIDMinSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6PIDMinSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7PIDMinSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8PIDMinSet()));
            break;
          case 8:
            fMapTecRefU.insert(make_pair(ix, pte));
            // QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tecRefUSet()));
            if (1 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec1RefUSet()));
            if (2 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec2RefUSet()));
            if (3 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec3RefUSet()));
            if (4 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec4RefUSet()));
            if (5 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec5RefUSet()));
            if (6 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec6RefUSet()));
            if (7 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec7RefUSet()));
            if (8 == ix) QObject::connect(pte, SIGNAL(returnPressed()), this, SLOT(tec8RefUSet()));
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
void TECExpert::tecSetFromUI(int itec, std::string rname, QLineEdit *ql) {
  QString sval = ql->text();
  float xval = sval.toFloat();
  cout << "xval = " << xval << " itec = " << itec << endl;
  fThread->setTECRegister(itec, rname, xval);
}


//// -------------------------------------------------------------------------------
//void TECExpert::tecVoltageSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "ControlVoltage_Set", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecModeSet() {
//  for(auto it: fMapTecMode) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "Mode", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecPIDkpSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "PID_kp", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecPIDkiSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "PID_ki", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecPIDkdSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "PID_kd", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecPIDMaxSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "PID_Max", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecPIDMinSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "PID_Min", xval);
//  }
//}


//// -------------------------------------------------------------------------------
//void TECExpert::tecRefUSet() {
//  for(auto it: fMapTecControlVoltageSet) {
//    QString sval = it.second->text();
//    float xval = sval.toFloat();
//    cout << "xval = " << xval << " it.first = " << it.first << endl;
//    fThread->setTECRegister(it.first, "Ref_U", xval);
//  }
//}
