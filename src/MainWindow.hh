#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFont>

#include "tLog.hh"
#include "driveHardware.hh"

class QLineEdit;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(tLog &x, driveHardware *h, QWidget *parent = nullptr);
    ~MainWindow();


signals:
  void signalQuitProgram();
  void signalValve(int);

public slots:
  void updateHardwareDisplay();

private:
  void btnQuit();
  void btnValve0();
  void btnValve1();

  void setupQLE(QLineEdit *);
  void setupLBL(QLabel *);

  QLineEdit *fqleCANbusErrors, *fqleI2CErrors, *fqleRunTime, *fqleVersion;
  QLineEdit *fqleWT, *fqleAT, *fqleRH, *fqleDP;

  QPushButton *fbtnValve0, *fbtnValve1;

  tLog&         fLOG;
  driveHardware *fpHw;

  QFont fFont1;

};
#endif // MAINWINDOW_H
