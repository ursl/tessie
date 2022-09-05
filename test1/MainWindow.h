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


private slots:
    void on_buttonQuit_clicked();
    void appendText(QString line);
    QString getTimeString();

    void on_buttonValve0_clicked();

    void on_checkBoxTEC0_clicked(bool checked);

    void on_checkboxTECLockAll_clicked(bool checked);

private:

    driveHardware fThread;
    tLog&         fLOG;

    // -- without the following line you cannot 'go to slot' in the UI designer
    //    (and this line requires the above include)
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
