#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"

#include "driveHardware.hh"
#include "tLog.hh"
#include "TECDisplay.h"

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

private:
    void appendText(QString line);
    QString getTimeString();

private slots:

    void clkValve0();
    void clkValve1();
    void clkValveAll();

    void clkRefresh();

    // -- I don't know how to change this in Qtcreator ...
    void checkTEC1(bool checked) {setCheckBoxTEC(1, checked); }
    void checkTEC2(bool checked) {setCheckBoxTEC(2, checked); }
    void checkTEC3(bool checked) {setCheckBoxTEC(3, checked); }
    void checkTEC4(bool checked) {setCheckBoxTEC(4, checked); }
    void checkTEC5(bool checked) {setCheckBoxTEC(5, checked); }
    void checkTEC6(bool checked) {setCheckBoxTEC(6, checked); }
    void checkTEC7(bool checked) {setCheckBoxTEC(7, checked); }
    void checkTEC8(bool checked) {setCheckBoxTEC(8, checked); }

    void checkTECAll(bool checked);

    void guiSetCanID();
    void guiSetRegName();
    void guiSetRegValue();
    void guiWriteToCAN();
    void guiReadFromCAN();

   // void updateVoltageValue();

    void tecSetFromUI(int itec, std::string rname, QWidget *);

    void tec8VoltSet() {tecSetFromUI(8, "ControlVoltage_Set", ui->tec8_Voltage); }
    void tec7VoltSet() {tecSetFromUI(7, "ControlVoltage_Set", ui->tec7_Voltage); }
    void tec6VoltSet() {tecSetFromUI(6, "ControlVoltage_Set", ui->tec6_Voltage); }
    void tec5VoltSet() {tecSetFromUI(5, "ControlVoltage_Set", ui->tec5_Voltage); }
    void tec4VoltSet() {tecSetFromUI(4, "ControlVoltage_Set", ui->tec4_Voltage); }
    void tec3VoltSet() {tecSetFromUI(3, "ControlVoltage_Set", ui->tec3_Voltage); }
    void tec2VoltSet() {tecSetFromUI(2, "ControlVoltage_Set", ui->tec2_Voltage); }
    void tec1VoltSet() {tecSetFromUI(1, "ControlVoltage_Set", ui->tec1_Voltage); }

    void tec8TempSet() {tecSetFromUI(8, "Temp_Set", ui->tec8_Temp); }
    void tec7TempSet() {tecSetFromUI(7, "Temp_Set", ui->tec7_Temp); }
    void tec6TempSet() {tecSetFromUI(6, "Temp_Set", ui->tec6_Temp); }
    void tec5TempSet() {tecSetFromUI(5, "Temp_Set", ui->tec5_Temp); }
    void tec4TempSet() {tecSetFromUI(4, "Temp_Set", ui->tec4_Temp); }
    void tec3TempSet() {tecSetFromUI(3, "Temp_Set", ui->tec3_Temp); }
    void tec2TempSet() {tecSetFromUI(2, "Temp_Set", ui->tec2_Temp); }
    void tec1TempSet() {tecSetFromUI(1, "Temp_Set", ui->tec1_Temp); }

    void tec8PIDkp() {tecSetFromUI(8, "PID_kp", ui->tec8_PID_kp); }
    void tec7PIDkp() {tecSetFromUI(7, "PID_kp", ui->tec7_PID_kp); }
    void tec6PIDkp() {tecSetFromUI(6, "PID_kp", ui->tec6_PID_kp); }
    void tec5PIDkp() {tecSetFromUI(5, "PID_kp", ui->tec5_PID_kp); }
    void tec4PIDkp() {tecSetFromUI(4, "PID_kp", ui->tec4_PID_kp); }
    void tec3PIDkp() {tecSetFromUI(3, "PID_kp", ui->tec3_PID_kp); }
    void tec2PIDkp() {tecSetFromUI(2, "PID_kp", ui->tec2_PID_kp); }
    void tec1PIDkp() {tecSetFromUI(1, "PID_kp", ui->tec1_PID_kp); }

    void tec8PIDki() {tecSetFromUI(8, "PID_ki", ui->tec8_PID_ki); }
    void tec7PIDki() {tecSetFromUI(7, "PID_ki", ui->tec7_PID_ki); }
    void tec6PIDki() {tecSetFromUI(6, "PID_ki", ui->tec6_PID_ki); }
    void tec5PIDki() {tecSetFromUI(5, "PID_ki", ui->tec5_PID_ki); }
    void tec4PIDki() {tecSetFromUI(4, "PID_ki", ui->tec4_PID_ki); }
    void tec3PIDki() {tecSetFromUI(3, "PID_ki", ui->tec3_PID_ki); }
    void tec2PIDki() {tecSetFromUI(2, "PID_ki", ui->tec2_PID_ki); }
    void tec1PIDki() {tecSetFromUI(1, "PID_ki", ui->tec1_PID_ki); }

    void tec8PIDkd() {tecSetFromUI(8, "PID_kd", ui->tec8_PID_kd); }
    void tec7PIDkd() {tecSetFromUI(7, "PID_kd", ui->tec7_PID_kd); }
    void tec6PIDkd() {tecSetFromUI(6, "PID_kd", ui->tec6_PID_kd); }
    void tec5PIDkd() {tecSetFromUI(5, "PID_kd", ui->tec5_PID_kd); }
    void tec4PIDkd() {tecSetFromUI(4, "PID_kd", ui->tec4_PID_kd); }
    void tec3PIDkd() {tecSetFromUI(3, "PID_kd", ui->tec3_PID_kd); }
    void tec2PIDkd() {tecSetFromUI(2, "PID_kd", ui->tec2_PID_kd); }
    void tec1PIDkd() {tecSetFromUI(1, "PID_kd", ui->tec1_PID_kd); }

    void tec8TempM() {tecSetFromUI(8, "Temp_M", ui->tec8_TempM); }
    void tec7TempM() {tecSetFromUI(7, "Temp_M", ui->tec7_TempM); }
    void tec6TempM() {tecSetFromUI(6, "Temp_M", ui->tec6_TempM); }
    void tec5TempM() {tecSetFromUI(5, "Temp_M", ui->tec5_TempM); }
    void tec4TempM() {tecSetFromUI(4, "Temp_M", ui->tec4_TempM); }
    void tec3TempM() {tecSetFromUI(3, "Temp_M", ui->tec3_TempM); }
    void tec2TempM() {tecSetFromUI(2, "Temp_M", ui->tec2_TempM); }
    void tec1TempM() {tecSetFromUI(1, "Temp_M", ui->tec1_TempM); }

    void openTECDisplay(int itec);
    void openTEC8() {openTECDisplay(8);}
    void openTEC7() {openTECDisplay(7);}
    void openTEC6() {openTECDisplay(6);}
    void openTEC5() {openTECDisplay(5);}
    void openTEC4() {openTECDisplay(4);}
    void openTEC3() {openTECDisplay(3);}
    void openTEC2() {openTECDisplay(2);}
    void openTEC1() {openTECDisplay(1);}

    void  updateHardwareValues();
    void  updateHardwareDisplay();

    void closeTECDisplay();

    void start();
    void quitProgram();

private:
    // -- without the following line you cannot 'go to slot'
    //    in the UI designer
    //    (and this line requires the above include)
    Ui::MainWindow *ui;

    tLog&         fLOG;
    driveHardware fThread;

    std::string fGuiRegName;
    int         fGuiTecId;
    float       fGuiRegValue;

    std::vector<QLineEdit*> fUIControlVoltageSet,
    fUITemp_Set,
    fUITempM,
    fUIPIDkp,
    fUIPIDki,
    fUIPIDkd;

    TECDisplay *fTECDisplay;

    std::vector<QCheckBox*> fUICheckBox;
};
#endif // MAINWINDOW_H
