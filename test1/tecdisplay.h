#ifndef TECDISPLAY_H
#define TECDISPLAY_H

#include <QWidget>

#include "driveHardware.hh"

// -------------------------------------------------------------------------------
namespace Ui {
  class TECDisplay;
}

// -------------------------------------------------------------------------------
class TECDisplay : public QWidget {
  Q_OBJECT

public:
  explicit TECDisplay(QWidget *parent = nullptr);
  ~TECDisplay();
  void setTitle(int itec);
  void setHardware(driveHardware *);
  void updateHardwareDisplay();

private:
  Ui::TECDisplay *ui;
  int fTECDisplayItec;
  driveHardware *fThread;
};

#endif // TECDISPLAY_H
