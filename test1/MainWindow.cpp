#include "MainWindow.h"

#include <sstream>

#include <qpushbutton.h>
#include <unistd.h>

#include "tLog.hh"

using namespace std;

// -------------------------------------------------------------------------------
MainWindow::MainWindow(tLog &x, driveHardware *h, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  fLOG(x), fpHw(h) {
  ui->setupUi(this);


  fTECDisplay  = new TECDisplay(this);
  fTECDisplay->close();
  fTECDisplay->setHardware(fpHw);

  ui->labelVersion->setText("2023/07/19-01");
  //ui->labelStatus->setText("OK");

  ui->lineEditRunTime->setAlignment(Qt::AlignRight);
  ui->lineEditCANbusError->setAlignment(Qt::AlignRight);
  ui->lineEditI2CError->setAlignment(Qt::AlignRight);
  ui->lineEditTemp->setAlignment(Qt::AlignRight);
  ui->lineEditRH->setAlignment(Qt::AlignRight);
  ui->lineEditDP->setAlignment(Qt::AlignRight);

  fUICheckBox.push_back(ui->checkBoxTEC1);
  fUICheckBox.push_back(ui->checkBoxTEC2);
  fUICheckBox.push_back(ui->checkBoxTEC3);
  fUICheckBox.push_back(ui->checkBoxTEC4);
  fUICheckBox.push_back(ui->checkBoxTEC5);
  fUICheckBox.push_back(ui->checkBoxTEC6);
  fUICheckBox.push_back(ui->checkBoxTEC7);
  fUICheckBox.push_back(ui->checkBoxTEC8);

  fUITemp_Set.push_back(ui->tec1_Temp);
  fUITemp_Set.push_back(ui->tec2_Temp);
  fUITemp_Set.push_back(ui->tec3_Temp);
  fUITemp_Set.push_back(ui->tec4_Temp);
  fUITemp_Set.push_back(ui->tec5_Temp);
  fUITemp_Set.push_back(ui->tec6_Temp);
  fUITemp_Set.push_back(ui->tec7_Temp);
  fUITemp_Set.push_back(ui->tec8_Temp);

  fUITempM.push_back(ui->tec1_TempM);
  fUITempM.push_back(ui->tec2_TempM);
  fUITempM.push_back(ui->tec3_TempM);
  fUITempM.push_back(ui->tec4_TempM);
  fUITempM.push_back(ui->tec5_TempM);
  fUITempM.push_back(ui->tec6_TempM);
  fUITempM.push_back(ui->tec7_TempM);
  fUITempM.push_back(ui->tec8_TempM);


  fUIControlVoltageSet.push_back(ui->tec1_Voltage);
  fUIControlVoltageSet.push_back(ui->tec2_Voltage);
  fUIControlVoltageSet.push_back(ui->tec3_Voltage);
  fUIControlVoltageSet.push_back(ui->tec4_Voltage);
  fUIControlVoltageSet.push_back(ui->tec5_Voltage);
  fUIControlVoltageSet.push_back(ui->tec6_Voltage);
  fUIControlVoltageSet.push_back(ui->tec7_Voltage);
  fUIControlVoltageSet.push_back(ui->tec8_Voltage);

  fUIMode.push_back(ui->tec1_Mode);
  fUIMode.push_back(ui->tec2_Mode);
  fUIMode.push_back(ui->tec3_Mode);
  fUIMode.push_back(ui->tec4_Mode);
  fUIMode.push_back(ui->tec5_Mode);
  fUIMode.push_back(ui->tec6_Mode);
  fUIMode.push_back(ui->tec7_Mode);
  fUIMode.push_back(ui->tec8_Mode);

  fUISupply_U.push_back(ui->tec1_Supply_U);
  fUISupply_U.push_back(ui->tec2_Supply_U);
  fUISupply_U.push_back(ui->tec3_Supply_U);
  fUISupply_U.push_back(ui->tec4_Supply_U);
  fUISupply_U.push_back(ui->tec5_Supply_U);
  fUISupply_U.push_back(ui->tec6_Supply_U);
  fUISupply_U.push_back(ui->tec7_Supply_U);
  fUISupply_U.push_back(ui->tec8_Supply_U);

  ui->buttonValve0->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  ui->buttonValve1->setStyleSheet("QPushButton {background-color: gray; color: black;}");

}


// ----------------------------------------------------------------------
MainWindow::~MainWindow() {
  delete ui;
}


// ----------------------------------------------------------------------
void MainWindow::appendText(QString line) {
  ui->textEditLog->append(line);
}


// ----------------------------------------------------------------------
void MainWindow::setBackground(QString name, QString color) {
  QString cstring = "QLineEdit {background-color : " + color + "; }";
  if (name == "T") {
    cout << "MainWindow::setBackground ui->lineEditTemp->setStyleSheet(" << cstring.toStdString() << endl;
    ui->lineEditTemp->setStyleSheet(cstring);
  } else if (name == "RH") {
    cout << "MainWindow::setBackground ui->lineEditRH->setStyleSheet(" << cstring.toStdString() << endl;
    ui->lineEditRH->setStyleSheet(cstring);
  } else if (name == "DP") {
    cout << "MainWindow::setBackground ui->lineEditDP->setStyleSheet(" << cstring.toStdString() << endl;
    ui->lineEditDP->setStyleSheet(cstring);
  } else if (name.contains("tec")) {
    name.remove(0, 3);
    int itec = name.toInt();
    cout << "MainWindow::setBackground fUITempM[" << itec-1 << "]->setStyleSheet("
         << cstring.toStdString() << endl;
    fUITempM[itec-1]->setStyleSheet(cstring);
  }

}


// ----------------------------------------------------------------------
void MainWindow::printText(string line) {
  ui->textEditLog->append(QString::fromStdString(line));
}


// ----------------------------------------------------------------------
QString MainWindow::getTimeString() {
  QString text = QString::fromStdString(fLOG.timeStamp());
  return text;
}


// ----------------------------------------------------------------------
void MainWindow::setCheckBoxTEC(int itec, bool state) {
  cout << "MainWindow::setCheckBoxTEC(" << itec << "," << state << ")" << endl;
  fUICheckBox[itec-1]->setChecked(state);
  if (state) {
//    emit signalTurnOnTEC(itec);
    fpHw->turnOnTEC(itec);
  } else {
//    emit signalTurnOffTEC(itec);
    fpHw->turnOffTEC(itec);
  }

}


// ----------------------------------------------------------------------
void MainWindow::quitProgram() {
  stringstream sbla; sbla << "This is the end, my friend";
  fLOG(INFO, "This is the end, my friend -- tessie shutting down");
  exit(0);
}


// ----------------------------------------------------------------------
void MainWindow::clkRefresh() {
  updateHardwareDisplay();

}

// ----------------------------------------------------------------------
void MainWindow::start() {
  stringstream sbla; sbla << "Startup";
  fLOG(INFO, sbla.str());
  // fThread.runPrintout(1,1);
}


// ----------------------------------------------------------------------
void MainWindow::tecSetFromUI(int itec, std::string rname, QWidget *qw) {
  QLineEdit *qle = (QLineEdit*)qw;
  qle->setStyleSheet("QLineEdit {color : red; }");
  QString sval = qle->text();
  float xval = sval.toFloat();
  fpHw->setTECRegister(itec, rname, xval);
  qle->clearFocus();
}


// ----------------------------------------------------------------------
void MainWindow::clkValve0() {
  // -- negate!
  if (!fpHw->getStatusValve0()) {
    ui->buttonValve0->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    ui->buttonValve0->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }

  emit signalValve(1);
}


// ----------------------------------------------------------------------
void MainWindow::clkValve1() {
  // -- negate!
  if (!fpHw->getStatusValve1()) {
    ui->buttonValve1->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    ui->buttonValve1->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }
  emit signalValve(2);
}


// ----------------------------------------------------------------------
void MainWindow::checkTECAll(bool checked) {
  stringstream sbla; sbla << "checkTECTAll clicked " << checked;
  fLOG(INFO, sbla.str());
  for (int i = 1; i <= 8; ++i) {
    setCheckBoxTEC(i, checked);
  }
}


// ----------------------------------------------------------------------
void MainWindow::guiWriteToCAN() {

  stringstream sbla; sbla << "writeToCAN: CAN ID = " << fGuiTecId
                          << " register ->" << fGuiRegName << "<-"
                          << " value = " << fGuiRegValue
                         ;

  fLOG(INFO, sbla.str());
  fpHw->setTECRegister(fGuiTecId, fGuiRegName, fGuiRegValue);
}


// ----------------------------------------------------------------------
void MainWindow::guiReadFromCAN() {
  float fval = fpHw->getTECRegisterFromCAN(fGuiTecId, fGuiRegName);
  ui->textTECValue->setText(QString::number(fval, 'f', 2));
}


// ----------------------------------------------------------------------
void MainWindow::guiSetCanID() {
  QString qline = ui->textTECNumber->toPlainText();
  int ibla = qline.toInt();

  stringstream sbla; sbla << "GUI changed CAN ID ->" << qline.toStdString().c_str() << "<- in int: " << ibla;
  fGuiTecId = ibla;
  fLOG(INFO, sbla.str());
}


// ----------------------------------------------------------------------
void MainWindow::guiSetRegValue() {
  QString qline = ui->textTECValue->toPlainText();
  float fbla = qline.toFloat();

  stringstream sbla; sbla << "GUI changed reg value ->" << qline.toStdString().c_str() << "<- in int: " << fbla;
  fGuiRegValue = fbla;
  fLOG(INFO, sbla.str());
}


// ----------------------------------------------------------------------
void MainWindow::guiSetRegName() {
  QString qline = ui->textTECName->toPlainText();
  string ssbla = qline.toStdString();

  stringstream sbla; sbla << "GUI changed reg name ->" << qline.toStdString().c_str() << "<- in string: " << ssbla;
  fGuiRegName = ssbla;
  fLOG(INFO, sbla.str());
}


// ----------------------------------------------------------------------
void MainWindow::updateHardwareDisplay() {
  if (0) cout << "MainWindow::updateHardwareDisplay() entered, fpHw->getRunCnt() = "
              << fpHw->getRunCnt()
              << endl;

  if (fTECDisplay->isVisible()) {
    fTECDisplay->updateHardwareDisplay();
  }

  ui->lineEditTemp->setText(QString::number(fpHw->getTemperature(), 'f', 2));
  ui->lineEditRH->setText(QString::number(fpHw->getRH(), 'f', 2));
  ui->lineEditDP->setText(QString::number(fpHw->getDP(), 'f', 2));
  ui->lineEditCANbusError->setText(QString::number(fpHw->getNCANbusErrors()));
  ui->lineEditCANbusError->setStyleSheet("QLineEdit {background-color : red; }");
  if (fpHw->redCANErrors() > 0) {
    cout << "Setting CANbus error counter line edit to red" << endl;
    ui->lineEditCANbusError->setStyleSheet("QLineEdit {background-color : red; }");
  } else {
    ui->lineEditCANbusError->setStyleSheet("QLineEdit {background-color : white; }");
  }
  ui->lineEditI2CError->setText(QString::number(fpHw->getNI2CErrors()));
  if (fpHw->redI2CErrors() > 0) {
    cout << "Setting I2C error counter line edit to red" << endl;
    ui->lineEditI2CError->setStyleSheet("QLineEdit {background-color : red; }");
  } else {
    ui->lineEditI2CError->setStyleSheet("QLineEdit {background-color : white; }");
  }

  ui->lineEditRunTime->setText(QString::number(fpHw->getRunTime()));

  if (fpHw->getStatusValve0()) {
    ui->buttonValve0->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    ui->buttonValve0->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }
  if (fpHw->getStatusValve1()) {
    ui->buttonValve1->setStyleSheet("QPushButton {background-color: #A3C1DA; color: black;}");
  } else {
    ui->buttonValve1->setStyleSheet("QPushButton {background-color: gray; color: black;}");
  }

  for (unsigned int ivec = 0; ivec < 8; ++ivec) {
    if (!fUIControlVoltageSet[ivec]->hasFocus()) {
      fUIControlVoltageSet[ivec]->setText(QString::number(fpHw->getTECRegister(ivec+1, "ControlVoltage_Set"), 'f', 2));
      fUIControlVoltageSet[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUITemp_Set[ivec]->hasFocus()) {
      fUITemp_Set[ivec]->setText(QString::number(fpHw->getTECRegister(ivec+1, "Temp_Set"), 'f', 2));
      fUITemp_Set[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUIMode[ivec]->hasFocus()) {
      fUIMode[ivec]->setText(QString::number(fpHw->getTECRegister(ivec+1, "Mode")));
      fUIMode[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUISupply_U[ivec]->hasFocus()) {
      fUISupply_U[ivec]->setText(QString::number(fpHw->getTECRegister(ivec+1, "Supply_U"), 'f', 2));
      fUISupply_U[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }

    fUITempM[ivec]->setText(QString::number(fpHw->getTECRegister(ivec+1, "Temp_M"), 'f', 2));
    fUITempM[ivec]->setStyleSheet("QLineEdit {color : green; }");

    fUISupply_U[ivec]->setText(QString::number(fpHw->getTECRegister(ivec+1, "Supply_U"), 'f', 2));

    bool state(false);
    if (fpHw->getTECRegister(ivec+1, "PowerState") > 0.5) {
      state = true;
    }

    fUICheckBox[ivec]->setChecked(state);
  }
}

// ----------------------------------------------------------------------
void MainWindow::openTECDisplay(int itec) {
  fLOG(INFO, stringstream("openTEC(" + to_string(itec) + ")").str());
  if (fTECDisplay->isVisible()) {
      fTECDisplay->updateHardwareDisplay();
  }
  fTECDisplay->setTitle(itec);
  fTECDisplay->show();
}

// ----------------------------------------------------------------------
void MainWindow::closeTECDisplay() {
  cout << "closeTECDisplay()"  << endl;
  fTECDisplay->close();
}



