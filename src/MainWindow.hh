#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFont>
#include <QPalette>

#include "tLog.hh"
#include "driveHardware.hh"

#include <vector>

class QLineEdit;
class QLabel;
class QPushButton;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(tLog &x, driveHardware *h, QWidget *parent = nullptr);
    ~MainWindow();


signals:
  void signalQuitProgram();
  void signalStopOperations();
  void signalValve(int);

public slots:
  void updateHardwareDisplay();
  void showAlarm();

private:
  void btnQuit();
  void btnStop();
  void btnValve0();
  void btnValve1();

  void setupQLE(QLineEdit *);
  void setupLBL(QLabel *);
  void mkTEC(int i);
  int  colorIndex(double t);
  int  colorReducedIndex(double t);

  QWidget *fWdg;

  QLineEdit *fqleCANbusErrors, *fqleI2CErrors, *fqleRunTime, *fqleStatus;
  QLineEdit *fqleWT, *fqleAT, *fqleRH, *fqleDP, *fqleLS, *fqleIL;

  QPushButton *fbtnValve0, *fbtnValve1;

  tLog&         fLOG;
  driveHardware *fpHw;

  QFont fFont1, fFont2;

  std::vector<QLineEdit* > fqleTEC;
  std::vector<QLabel* >    flblTEC;

  std::vector<QPalette>    fPalettes;

  bool fAlarmSoundPlaying{false};
  
};
#endif // MAINWINDOW_H
