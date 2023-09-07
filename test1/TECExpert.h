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
  void tecVoltSet();

private:
  driveHardware *fThread;
  QWidget *fUI;
  MainWindow *fMW;
  std::map<int, QLineEdit*> fMapTecMode, fMapTecControlVoltageSet;
};

#endif // TECEXPERT_H
