#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"

#include "driveHardware.hh"
#include "TECDisplay.h"
#include "tLog.hh"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(tLog &x, driveHardware *h, QWidget *parent = nullptr);
    ~MainWindow();

    void printText(std::string line);
    void setCheckBoxTEC(int itec, bool state);

    void openTECDisplay(int itec);
    void closeTECDisplay();

public slots:
    void appendText(QString line);
    void updateHardwareDisplay();
    void setBackground(QString name, QString color);
    void showAlarm();

    void start();
    void quitProgram();


signals:
    void signalValve(int);
    void signalTurnOnTEC(int);
    void signalTurnOffTEC(int);
    void signalReadCAN();

private slots:
    QString getTimeString();

    void clkValve0();
    void clkValve1();

    void clkRefresh();

    // -- I don't know how to change this into using arrays/vectors in QtCreator ...
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

    void tec8Supply_U() {tecSetFromUI(8, "Supply_U", ui->tec8_Supply_U); }
    void tec7Supply_U() {tecSetFromUI(7, "Supply_U", ui->tec7_Supply_U); }
    void tec6Supply_U() {tecSetFromUI(6, "Supply_U", ui->tec6_Supply_U); }
    void tec5Supply_U() {tecSetFromUI(5, "Supply_U", ui->tec5_Supply_U); }
    void tec4Supply_U() {tecSetFromUI(4, "Supply_U", ui->tec4_Supply_U); }
    void tec3Supply_U() {tecSetFromUI(3, "Supply_U", ui->tec3_Supply_U); }
    void tec2Supply_U() {tecSetFromUI(2, "Supply_U", ui->tec2_Supply_U); }
    void tec1Supply_U() {tecSetFromUI(1, "Supply_U", ui->tec1_Supply_U); }

    void tec8Mode() {tecSetFromUI(8, "Mode", ui->tec8_Mode); }
    void tec7Mode() {tecSetFromUI(7, "Mode", ui->tec7_Mode); }
    void tec6Mode() {tecSetFromUI(6, "Mode", ui->tec6_Mode); }
    void tec5Mode() {tecSetFromUI(5, "Mode", ui->tec5_Mode); }
    void tec4Mode() {tecSetFromUI(4, "Mode", ui->tec4_Mode); }
    void tec3Mode() {tecSetFromUI(3, "Mode", ui->tec3_Mode); }
    void tec2Mode() {tecSetFromUI(2, "Mode", ui->tec2_Mode); }
    void tec1Mode() {tecSetFromUI(1, "Mode", ui->tec1_Mode); }

    void tec8TempM() {tecSetFromUI(8, "Temp_M", ui->tec8_TempM); }
    void tec7TempM() {tecSetFromUI(7, "Temp_M", ui->tec7_TempM); }
    void tec6TempM() {tecSetFromUI(6, "Temp_M", ui->tec6_TempM); }
    void tec5TempM() {tecSetFromUI(5, "Temp_M", ui->tec5_TempM); }
    void tec4TempM() {tecSetFromUI(4, "Temp_M", ui->tec4_TempM); }
    void tec3TempM() {tecSetFromUI(3, "Temp_M", ui->tec3_TempM); }
    void tec2TempM() {tecSetFromUI(2, "Temp_M", ui->tec2_TempM); }
    void tec1TempM() {tecSetFromUI(1, "Temp_M", ui->tec1_TempM); }

    void openTEC8() {openTECDisplay(8);}
    void openTEC7() {openTECDisplay(7);}
    void openTEC6() {openTECDisplay(6);}
    void openTEC5() {openTECDisplay(5);}
    void openTEC4() {openTECDisplay(4);}
    void openTEC3() {openTECDisplay(3);}
    void openTEC2() {openTECDisplay(2);}
    void openTEC1() {openTECDisplay(1);}

private:
    // -- without the following line you cannot 'go to slot'
    //    in the UI designer
    //    (and this line requires the above include)
    Ui::MainWindow *ui;

    tLog&         fLOG;
    driveHardware *fpHw;
    TECDisplay    *fTECDisplay;

    std::string fGuiRegName;
    int         fGuiTecId;
    float       fGuiRegValue;

    std::vector<QLineEdit*> fUIControlVoltageSet,
    fUITemp_Set,
    fUITempM,
    fUISupply_U,
    fUIMode,
    fUIPIDkd;

    std::vector<QCheckBox*> fUICheckBox;
};
#endif // MAINWINDOW_H
