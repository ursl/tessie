#ifndef TECEXPERT_H
#define TECEXPERT_H

#include <QWidget>
#include <QLineEdit>


#include "driveHardware.hh"

// -------------------------------------------------------------------------------
class TECExpert : public QWidget {
  Q_OBJECT

public:
  explicit TECExpert(MainWindow *parent = nullptr, driveHardware *x = 0);
  ~TECExpert();
  void setHardware(driveHardware *);
  void close();

  void tecSetFromUI(int itec, std::string rname, QLineEdit *);

public slots:
  void tec1VoltageSet() {tecSetFromUI(1, "ControlVoltage_Set", fMapTecControlVoltageSet[1]); }
  void tec2VoltageSet() {tecSetFromUI(2, "ControlVoltage_Set", fMapTecControlVoltageSet[2]); }
  void tec3VoltageSet() {tecSetFromUI(3, "ControlVoltage_Set", fMapTecControlVoltageSet[3]); }
  void tec4VoltageSet() {tecSetFromUI(4, "ControlVoltage_Set", fMapTecControlVoltageSet[4]); }
  void tec5VoltageSet() {tecSetFromUI(5, "ControlVoltage_Set", fMapTecControlVoltageSet[5]); }
  void tec6VoltageSet() {tecSetFromUI(6, "ControlVoltage_Set", fMapTecControlVoltageSet[6]); }
  void tec7VoltageSet() {tecSetFromUI(7, "ControlVoltage_Set", fMapTecControlVoltageSet[7]); }
  void tec8VoltageSet() {tecSetFromUI(8, "ControlVoltage_Set", fMapTecControlVoltageSet[8]); }


  void tecModeSet();
  void tecPIDkpSet();
  void tecPIDkiSet();
  void tecPIDkdSet();
  void tecPIDMaxSet();
  void tecPIDMinSet();
  void tecRefUSet();

private:
  driveHardware *fThread;
  QWidget *fUI;
  MainWindow *fMW;
  std::map<int, QLineEdit*> fMapTecMode,
    fMapTecControlVoltageSet,
    fMapTecPIDkp,  fMapTecPIDki,  fMapTecPIDkd,
    fMapTecPIDMax, fMapTecPIDMin, fMapTecRefU;
};

#endif // TECEXPERT_H
