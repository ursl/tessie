#ifndef TECEXPERT_H
#define TECEXPERT_H

#include <QWidget>

#include "driveHardware.hh"

// -------------------------------------------------------------------------------
class TECExpert : public QWidget {
  Q_OBJECT

public:
  explicit TECExpert(QWidget *parent = nullptr, driveHardware *x = 0);
  ~TECExpert();
  void setHardware(driveHardware *);
  void close();

private:
  driveHardware *fThread;
  QWidget *fUI;
};

#endif // TECEXPERT_H
