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
  void signalStopOperations(int);
  void signalValve(int);

public slots:
  void updateHardwareDisplay();
  void showAlarm(int);

private:
  void btnQuit();
  void btnINTL();
  void btnSound();
  void btnStop();
  void btnRestartTessieWeb();
  void btnValve0();
  void btnValve1();

  void setupQLE(QLineEdit *);
  void setupLBL(QLabel *);
  void mkTEC(int i);
  int  colorIndex(double t);
  int  colorReducedIndex(double t);
  void killSiren();

  QWidget *fWdg;

  QLineEdit *fqleBusErrors, *fqleFlowSwitch, *fqleFreeDisk, *fqleRunTime, *fqleStatus;
  QLineEdit *fqleWT, *fqleAT, *fqleRH, *fqleDP, *fqleLS, *fqleIL;

  QPushButton *fbtnValve0, *fbtnValve1;

  tLog&         fLOG;
  driveHardware *fpHw;

  QFont fFont1, fFont2;

  std::vector<QLineEdit* > fqleTEC;
  std::vector<QLabel* >    flblTEC;

  std::vector<QPalette>    fPalettes;

  bool fAlarmSoundPlaying{false};

  std::map<std::string, QString> fStyleSheet;
  
};
#endif // MAINWINDOW_H
