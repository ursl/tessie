#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFont>

#include "tLog.hh"
#include "driveHardware.hh"

class QLineEdit;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(tLog &x, driveHardware *h, QWidget *parent = nullptr);
    ~MainWindow();


signals:
  void signalQuitProgram();

public slots:
  void updateHardwareDisplay();

private:
  void btnQuit();
  void setupQLE(QLineEdit *);
  void setupLBL(QLabel *);

  QLineEdit *fqleCANbusErrors, *fqleI2CErrors, *fqleRunTime, *fqleVersion;
  QLineEdit *fqleWT, *fqleAT, *fqleRH, *fqleDP;

  tLog&         fLOG;
  driveHardware *fpHw;

  QFont fFont1;

};
#endif // MAINWINDOW_H
