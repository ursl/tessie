#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "driveHardware.hh"
#include "tLog.hh"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(tLog &x, QWidget *parent = nullptr);
    ~MainWindow();

    void printText(std::string line);
    void setCheckBoxTEC(int itec, bool state);

private slots:
    void appendText(QString line);
    QString getTimeString();

    void checkValve0(bool checked);
    void checkValve1(bool checked);
    void checkValveAll(bool checked);


    void checkTEC0(bool checked);
    void checkTEC1(bool checked);
    void checkTEC2(bool checked);
    void checkTEC3(bool checked);
    void checkTEC4(bool checked);
    void checkTEC5(bool checked);
    void checkTEC6(bool checked);
    void checkTEC7(bool checked);
    void checkTECAll(bool checked);
    void quitProgram();

private:

    driveHardware fThread;
    tLog&         fLOG;

    // -- without the following line you cannot 'go to slot' in the UI designer
    //    (and this line requires the above include)
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
