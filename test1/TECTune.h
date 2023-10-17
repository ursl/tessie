#ifndef TECTUNE_H
#define TECTUNE_H

#include <QWidget>
#include <QLineEdit>


#include "driveHardware.hh"

// -------------------------------------------------------------------------------
class TECTune : public QWidget {
  Q_OBJECT

public:
  explicit TECTune(MainWindow *parent = nullptr, driveHardware *x = 0);
  ~TECTune();
  void setHardware(driveHardware *);
  void updateHardwareDisplay();
  void close();

  void tecSetFromUI(int itec, std::string rname, QLineEdit *);

public slots:


private:
  driveHardware *fThread;
  QWidget *fUI;
  MainWindow *fMW;

};

#endif // TECTUNE_H
