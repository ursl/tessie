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

public slots:
  void tecVoltageSet();
  void tecModeSet(int);
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
