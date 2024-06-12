#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "tLog.hh"
#include "driveHardware.hh"

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

};
#endif // MAINWINDOW_H
