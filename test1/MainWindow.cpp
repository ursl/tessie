#include "MainWindow.h"

#include <sstream>

#include <qpushbutton.h>
#include <unistd.h>


#include "tLog.hh"


using namespace std;

// -------------------------------------------------------------------------------
MainWindow::MainWindow(tLog &x, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  fLOG(x),
  fThread(x) {
  fLOG.setHw(&fThread);
  ui->setupUi(this);

  fTECDisplay  = new TECDisplay(this);
  fTECDisplay->close();
  fTECDisplay->setHardware(&fThread);

  ui->lineEditRunTime->setAlignment(Qt::AlignRight);
  ui->lineEditCANbusError->setAlignment(Qt::AlignRight);
  ui->lineEditI2CError->setAlignment(Qt::AlignRight);

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

  fUIPIDkp.push_back(ui->tec1_PID_kp);
  fUIPIDkp.push_back(ui->tec2_PID_kp);
  fUIPIDkp.push_back(ui->tec3_PID_kp);
  fUIPIDkp.push_back(ui->tec4_PID_kp);
  fUIPIDkp.push_back(ui->tec5_PID_kp);
  fUIPIDkp.push_back(ui->tec6_PID_kp);
  fUIPIDkp.push_back(ui->tec7_PID_kp);
  fUIPIDkp.push_back(ui->tec8_PID_kp);

  fUIPIDki.push_back(ui->tec1_PID_ki);
  fUIPIDki.push_back(ui->tec2_PID_ki);
  fUIPIDki.push_back(ui->tec3_PID_ki);
  fUIPIDki.push_back(ui->tec4_PID_ki);
  fUIPIDki.push_back(ui->tec5_PID_ki);
  fUIPIDki.push_back(ui->tec6_PID_ki);
  fUIPIDki.push_back(ui->tec7_PID_ki);
  fUIPIDki.push_back(ui->tec8_PID_ki);

  fUIPIDkd.push_back(ui->tec1_PID_kd);
  fUIPIDkd.push_back(ui->tec2_PID_kd);
  fUIPIDkd.push_back(ui->tec3_PID_kd);
  fUIPIDkd.push_back(ui->tec4_PID_kd);
  fUIPIDkd.push_back(ui->tec5_PID_kd);
  fUIPIDkd.push_back(ui->tec6_PID_kd);
  fUIPIDkd.push_back(ui->tec7_PID_kd);
  fUIPIDkd.push_back(ui->tec8_PID_kd);


  updateHardwareValues();

  connect(&fThread, &driveHardware::updateHwDisplay, this, &MainWindow::updateHardwareDisplay);
  connect(&fThread, &driveHardware::signalText, this, &MainWindow::appendText);
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
QString MainWindow::getTimeString() {
  QDateTime dati = QDateTime::currentDateTime();
  int seconds = dati.time().second();
  int minutes = dati.time().minute();
  int hours   = dati.time().hour();
  int year    = dati.date().year();
  int month   = dati.date().month();
  int day     = dati.date().day();
  QString text;
  text = QString("%1/%2/%3 %4:%5:%6")
      .arg(year, 4)
      .arg(month, 2, 10, QChar('0'))
      .arg(day, 2, 10, QChar('0'))
      .arg(hours, 2, 10, QChar('0'))
      .arg(minutes, 2, 10, QChar('0'))
      .arg(seconds, 2, 10, QChar('0'));
  return text;
}


// ----------------------------------------------------------------------
void MainWindow::setCheckBoxTEC(int itec, bool state) {
  fUICheckBox[itec-1]->setChecked(state);
  if (state) {
    fThread.turnOnTEC(itec);
  } else {
    fThread.turnOffTEC(itec);
  }

}


// ----------------------------------------------------------------------
void MainWindow::quitProgram() {
  stringstream sbla; sbla << "This is the end, my friend";
  ui->textEditLog->append(sbla.str().c_str());
  exit(0);
}


// ----------------------------------------------------------------------
void MainWindow::clkRefresh() {
  updateHardwareValues();

}

// ----------------------------------------------------------------------
void MainWindow::start() {
  stringstream sbla; sbla << "Startup";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.runPrintout(1,1);
}


// ----------------------------------------------------------------------
void MainWindow::tecSetFromUI(int itec, std::string rname, QWidget *qw) {
  QLineEdit *qle = (QLineEdit*)qw;
  qle->setStyleSheet("QLineEdit {color : red; }");
  QString sval = qle->text();
  float xval = sval.toFloat();
  fThread.setTECRegister(itec, rname, xval);
  qle->clearFocus();
}



// ----------------------------------------------------------------------
void MainWindow::clkValve0() {
  stringstream sbla; sbla << "checkValve0 clicked ";
  ui->textEditLog->append(sbla.str().c_str());
  sbla << "toggleFRAS(1)";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.toggleFras(1);
}


// ----------------------------------------------------------------------
void MainWindow::clkValve1() {
  stringstream sbla; sbla << "checkValve1 clicked ";
  ui->textEditLog->append(sbla.str().c_str());
  sbla << "toggleFRAS(2)";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.toggleFras(2);
}


// ----------------------------------------------------------------------
void MainWindow::clkValveAll() {
  stringstream sbla; sbla << "checkValveAll clicked ";
  ui->textEditLog->append(sbla.str().c_str());
  sbla << "toggleFRAS(3)";
  ui->textEditLog->append(sbla.str().c_str());
  fThread.toggleFras(3);
}



// ----------------------------------------------------------------------
void MainWindow::checkTECAll(bool checked) {
  stringstream sbla; sbla << "checkTECTAll clicked " << checked;
  string sline = sbla.str();
  QString qline = sline.c_str();
  ui->textEditLog->append(qline);
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

  fThread.setTECRegister(fGuiTecId, fGuiRegName, fGuiRegValue);

  ui->textEditLog->append(sbla.str().c_str());
}


// ----------------------------------------------------------------------
void MainWindow::guiReadFromCAN() {
  float fval = fThread.getTECRegisterFromCAN(fGuiTecId, fGuiRegName);
  ui->textTECValue->setText(QString::number(fval, 'f', 2));
}


// ----------------------------------------------------------------------
void MainWindow::guiSetCanID() {
  QString qline = ui->textTECNumber->toPlainText();
  int ibla = qline.toInt();

  stringstream sbla; sbla << "GUI changed CAN ID ->" << qline.toStdString().c_str() << "<- in int: " << ibla;
  fGuiTecId = ibla;
  ui->textEditLog->append(sbla.str().c_str());
}


// ----------------------------------------------------------------------
void MainWindow::guiSetRegValue() {
  QString qline = ui->textTECValue->toPlainText();
  float fbla = qline.toFloat();

  stringstream sbla; sbla << "GUI changed reg value ->" << qline.toStdString().c_str() << "<- in int: " << fbla;
  fGuiRegValue = fbla;
  ui->textEditLog->append(sbla.str().c_str());
}


// ----------------------------------------------------------------------
void MainWindow::guiSetRegName() {
  QString qline = ui->textTECName->toPlainText();
  string ssbla = qline.toStdString();

  stringstream sbla; sbla << "GUI changed reg name ->" << qline.toStdString().c_str() << "<- in string: " << ssbla;
  fGuiRegName = ssbla;
  ui->textEditLog->append(sbla.str().c_str());
}


// ----------------------------------------------------------------------
void MainWindow::updateHardwareValues() {
  fThread.readAllParamsFromCAN();
  updateHardwareDisplay();
}

// ----------------------------------------------------------------------
void MainWindow::updateHardwareDisplay() {

  if (fTECDisplay->isVisible()) {
    // cout << "MainWindow::updateHardwareDisplay() fTECDisplay->isVisible()" << endl;
    fTECDisplay->updateHardwareDisplay();
  }

  ui->lineEditTemp->setText(QString::number(fThread.getTemperature(), 'f', 2));
  ui->lineEditRH->setText(QString::number(fThread.getRH(), 'f', 2));
  ui->lineEditDP->setText(QString::number(fThread.getDP(), 'f', 2));
  ui->lineEditCANbusError->setText(QString::number(fThread.getNCANbusErrors()));
  ui->lineEditI2CError->setText(QString::number(fThread.getNI2CErrors()));
  ui->lineEditRunTime->setText(QString::number(fThread.getRunTime()));


  for (unsigned int ivec = 0; ivec < 8; ++ivec) {
    if (!fUIControlVoltageSet[ivec]->hasFocus()) {
      fUIControlVoltageSet[ivec]->setText(QString::number(fThread.getTECRegister(ivec+1, "ControlVoltage_Set"), 'f', 2));
      fUIControlVoltageSet[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUITemp_Set[ivec]->hasFocus()) {
      fUITemp_Set[ivec]->setText(QString::number(fThread.getTECRegister(ivec+1, "Temp_Set"), 'f', 2));
      fUITemp_Set[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUIPIDki[ivec]->hasFocus()) {
      fUIPIDki[ivec]->setText(QString::number(fThread.getTECRegister(ivec+1, "PID_ki"), 'f', 2));
      fUIPIDki[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUIPIDkp[ivec]->hasFocus()) {
      fUIPIDkp[ivec]->setText(QString::number(fThread.getTECRegister(ivec+1, "PID_kp"), 'f', 2));
      fUIPIDkp[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }
    if (!fUIPIDkd[ivec]->hasFocus()) {
      fUIPIDkd[ivec]->setText(QString::number(fThread.getTECRegister(ivec+1, "PID_kd"), 'f', 2));
      fUIPIDkd[ivec]->setStyleSheet("QLineEdit {color : green; }");
    }

    fUITempM[ivec]->setText(QString::number(fThread.getTECRegister(ivec+1, "Temp_M"), 'f', 2));
    fUITempM[ivec]->setStyleSheet("QLineEdit {color : green; }");

    bool state(false);
    if (fThread.getTECRegister(ivec+1, "PowerState") > 0.5) {
      state = true;
    }

    fUICheckBox[ivec]->setChecked(state);
  }
}

// ----------------------------------------------------------------------
void MainWindow::openTECDisplay(int itec) {
  cout << "openTEC(" << itec << ")" << endl;
  if (fTECDisplay->isVisible()) {
    updateHardwareValues();
  }
  fTECDisplay->setTitle(itec);
  fTECDisplay->show();
}

// ----------------------------------------------------------------------
void MainWindow::closeTECDisplay() {
  cout << "closeTECDisplay()"  << endl;
  fTECDisplay->close();
}
