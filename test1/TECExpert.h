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
  void tec1ModeSet() {tecSetFromUI(1, "Mode", fMapTecMode[1]); }
  void tec2ModeSet() {tecSetFromUI(2, "Mode", fMapTecMode[2]); }
  void tec3ModeSet() {tecSetFromUI(3, "Mode", fMapTecMode[3]); }
  void tec4ModeSet() {tecSetFromUI(4, "Mode", fMapTecMode[4]); }
  void tec5ModeSet() {tecSetFromUI(5, "Mode", fMapTecMode[5]); }
  void tec6ModeSet() {tecSetFromUI(6, "Mode", fMapTecMode[6]); }
  void tec7ModeSet() {tecSetFromUI(7, "Mode", fMapTecMode[7]); }
  void tec8ModeSet() {tecSetFromUI(8, "Mode", fMapTecMode[8]); }

  void tec1VoltageSet() {tecSetFromUI(1, "ControlVoltage_Set", fMapTecControlVoltageSet[1]); }
  void tec2VoltageSet() {tecSetFromUI(2, "ControlVoltage_Set", fMapTecControlVoltageSet[2]); }
  void tec3VoltageSet() {tecSetFromUI(3, "ControlVoltage_Set", fMapTecControlVoltageSet[3]); }
  void tec4VoltageSet() {tecSetFromUI(4, "ControlVoltage_Set", fMapTecControlVoltageSet[4]); }
  void tec5VoltageSet() {tecSetFromUI(5, "ControlVoltage_Set", fMapTecControlVoltageSet[5]); }
  void tec6VoltageSet() {tecSetFromUI(6, "ControlVoltage_Set", fMapTecControlVoltageSet[6]); }
  void tec7VoltageSet() {tecSetFromUI(7, "ControlVoltage_Set", fMapTecControlVoltageSet[7]); }
  void tec8VoltageSet() {tecSetFromUI(8, "ControlVoltage_Set", fMapTecControlVoltageSet[8]); }

  void tec1PIDkpSet() {tecSetFromUI(1, "PID_kp", fMapTecPIDkp[1]); }
  void tec2PIDkpSet() {tecSetFromUI(2, "PID_kp", fMapTecPIDkp[2]); }
  void tec3PIDkpSet() {tecSetFromUI(3, "PID_kp", fMapTecPIDkp[3]); }
  void tec4PIDkpSet() {tecSetFromUI(4, "PID_kp", fMapTecPIDkp[4]); }
  void tec5PIDkpSet() {tecSetFromUI(5, "PID_kp", fMapTecPIDkp[5]); }
  void tec6PIDkpSet() {tecSetFromUI(6, "PID_kp", fMapTecPIDkp[6]); }
  void tec7PIDkpSet() {tecSetFromUI(7, "PID_kp", fMapTecPIDkp[7]); }
  void tec8PIDkpSet() {tecSetFromUI(8, "PID_kp", fMapTecPIDkp[8]); }

  void tec1PIDkiSet() {tecSetFromUI(1, "PID_ki", fMapTecPIDki[1]); }
  void tec2PIDkiSet() {tecSetFromUI(2, "PID_ki", fMapTecPIDki[2]); }
  void tec3PIDkiSet() {tecSetFromUI(3, "PID_ki", fMapTecPIDki[3]); }
  void tec4PIDkiSet() {tecSetFromUI(4, "PID_ki", fMapTecPIDki[4]); }
  void tec5PIDkiSet() {tecSetFromUI(5, "PID_ki", fMapTecPIDki[5]); }
  void tec6PIDkiSet() {tecSetFromUI(6, "PID_ki", fMapTecPIDki[6]); }
  void tec7PIDkiSet() {tecSetFromUI(7, "PID_ki", fMapTecPIDki[7]); }
  void tec8PIDkiSet() {tecSetFromUI(8, "PID_ki", fMapTecPIDki[8]); }

  void tec1PIDkdSet() {tecSetFromUI(1, "PID_kd", fMapTecPIDkd[1]); }
  void tec2PIDkdSet() {tecSetFromUI(2, "PID_kd", fMapTecPIDkd[2]); }
  void tec3PIDkdSet() {tecSetFromUI(3, "PID_kd", fMapTecPIDkd[3]); }
  void tec4PIDkdSet() {tecSetFromUI(4, "PID_kd", fMapTecPIDkd[4]); }
  void tec5PIDkdSet() {tecSetFromUI(5, "PID_kd", fMapTecPIDkd[5]); }
  void tec6PIDkdSet() {tecSetFromUI(6, "PID_kd", fMapTecPIDkd[6]); }
  void tec7PIDkdSet() {tecSetFromUI(7, "PID_kd", fMapTecPIDkd[7]); }
  void tec8PIDkdSet() {tecSetFromUI(8, "PID_kd", fMapTecPIDkd[8]); }

  void tec1PIDMaxSet() {tecSetFromUI(1, "PID_Max", fMapTecPIDMax[1]); }
  void tec2PIDMaxSet() {tecSetFromUI(2, "PID_Max", fMapTecPIDMax[2]); }
  void tec3PIDMaxSet() {tecSetFromUI(3, "PID_Max", fMapTecPIDMax[3]); }
  void tec4PIDMaxSet() {tecSetFromUI(4, "PID_Max", fMapTecPIDMax[4]); }
  void tec5PIDMaxSet() {tecSetFromUI(5, "PID_Max", fMapTecPIDMax[5]); }
  void tec6PIDMaxSet() {tecSetFromUI(6, "PID_Max", fMapTecPIDMax[6]); }
  void tec7PIDMaxSet() {tecSetFromUI(7, "PID_Max", fMapTecPIDMax[7]); }
  void tec8PIDMaxSet() {tecSetFromUI(8, "PID_Max", fMapTecPIDMax[8]); }

  void tec1PIDMinSet() {tecSetFromUI(1, "PID_Min", fMapTecPIDMin[1]); }
  void tec2PIDMinSet() {tecSetFromUI(2, "PID_Min", fMapTecPIDMin[2]); }
  void tec3PIDMinSet() {tecSetFromUI(3, "PID_Min", fMapTecPIDMin[3]); }
  void tec4PIDMinSet() {tecSetFromUI(4, "PID_Min", fMapTecPIDMin[4]); }
  void tec5PIDMinSet() {tecSetFromUI(5, "PID_Min", fMapTecPIDMin[5]); }
  void tec6PIDMinSet() {tecSetFromUI(6, "PID_Min", fMapTecPIDMin[6]); }
  void tec7PIDMinSet() {tecSetFromUI(7, "PID_Min", fMapTecPIDMin[7]); }
  void tec8PIDMinSet() {tecSetFromUI(8, "PID_Min", fMapTecPIDMin[8]); }

  void tec1RefUSet() {tecSetFromUI(1, "Ref_U", fMapTecRefU[1]); }
  void tec2RefUSet() {tecSetFromUI(2, "Ref_U", fMapTecRefU[2]); }
  void tec3RefUSet() {tecSetFromUI(3, "Ref_U", fMapTecRefU[3]); }
  void tec4RefUSet() {tecSetFromUI(4, "Ref_U", fMapTecRefU[4]); }
  void tec5RefUSet() {tecSetFromUI(5, "Ref_U", fMapTecRefU[5]); }
  void tec6RefUSet() {tecSetFromUI(6, "Ref_U", fMapTecRefU[6]); }
  void tec7RefUSet() {tecSetFromUI(7, "Ref_U", fMapTecRefU[7]); }
  void tec8RefUSet() {tecSetFromUI(8, "Ref_U", fMapTecRefU[8]); }

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
