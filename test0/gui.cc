#include <iostream>
#include <sstream>
#include <array>

#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtWidgets/QScrollBar>

#include <QtCore/QDateTime>

#include <QtWidgets/QGraphicsLayout>

#include "gui.hh"

using namespace std;
using namespace QtCharts;

// ----------------------------------------------------------------------
void replaceAll(string &str, const string &from, const string &to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}


// ----------------------------------------------------------------------
// -- the following works on both macos and raspian
string getLoad() {
    std::array<char,128> buffer;
    std::string result, result2;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("/usr/bin/w | /usr/bin/head -n 1", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    string::size_type s1 = result.rfind("average");
    result.erase(0, s1+sizeof("average"));
    replaceAll(result, "\n", "");
    replaceAll(result, ": ", "");
    replaceAll(result, ":", "");
    replaceAll(result, ",", "");
    s1 = result.find(" ", 1);

    string resultNew = result;
    resultNew.erase(s1);
    replaceAll(resultNew, " ", "");
    string tmp = "result ->" + result + "<- resultNew ->" + resultNew + "<-";
    // return tmp;

    return resultNew;
}


// ----------------------------------------------------------------------
gui::gui(tLog &x, QMainWindow *parent): QMainWindow(parent), fLOG(x), fThread(x) {

    fLOG.setHw(&fThread);

    fDoUpdate = true;

    // -- connect the ui_gui (QML design) with the GUI class:
    ui = new Ui::MainWindow();
    ui->setupUi(this);
    updateTime();

    // -- set up a timer to display a clock
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &gui::updateTime);
    timer->start(1000);
    string welcome = fLOG.print(ALL, " *** Welcome to Tessie's *** ");


    ui->textEdit->setText(QString::fromStdString(welcome));
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());

    this->show();


    // -- set up display chart with timer
    double updateSec = 10;
    QTimer *timer2 = new QTimer(this);
    connect(timer2, &QTimer::timeout, this, &gui::updateCPULoad);
    timer2->start(1000*updateSec);


    QDateTime momentInTime = QDateTime::currentDateTime();
    fStartTime = momentInTime.toMSecsSinceEpoch();
    updateCPULoad();

    connect(&fThread, &driveHardware::signalText, this, &gui::appendText);
    connect(&fLOG, &tLog::signalText, this, &gui::appendText);

//    connect(ui->graphicsView, &timeChart::doUpdate, this, &gui::on_graphUpdate);

}

// ----------------------------------------------------------------------
gui::~gui() {
}

// ----------------------------------------------------------------------
void gui::printText(std::string line) {
    QString qline = QString::fromStdString(line);
    appendText(qline);
}


// ----------------------------------------------------------------------
void gui::updateTime() {
    ui->label_3->setText(getTimeString());
}

// ----------------------------------------------------------------------
void gui::updateCPULoad() {
    int ncounts = 100;
    float cpuload = ::atof(getLoad().c_str());
    QDateTime momentInTime = QDateTime::currentDateTime();

    fCurrent = (momentInTime.toMSecsSinceEpoch()-fStartTime)/1000.;
    ui->graphicsView->addPoint(static_cast<qreal>(fCurrent),
                               static_cast<qreal>(cpuload));

    string toprint = "cpu: " + std::to_string((momentInTime.toMSecsSinceEpoch()-fStartTime)/1000.) + ": " + std::to_string(cpuload);
    fLOG(ALL, toprint);
}

// ----------------------------------------------------------------------
void gui::appendText(QString line) {
    ui->textEdit->append(line);
}


// ----------------------------------------------------------------------
void gui::on_pushButton_clicked() {
    fLOG(INFO, "Start");
    fThread.runPrintout(1,1);
}

// ----------------------------------------------------------------------
void gui::on_pushButton_2_clicked() {
    fLOG(INFO, "Exit button clicked");
#ifdef PI
    fThread.shutDown();
#endif

    exit(0);
}

// ----------------------------------------------------------------------
void gui::on_pushButton_3_clicked() {
  fThread.setId(0x121);
  stringstream sbla; sbla << "CAN write ID = " << hex << fThread.getId()
                          << " reg = " << fThread.getRegister()
                          << " value = " << fThread.getValue();
  fLOG(INFO, sbla.str());
#ifdef PI
  fThread.sendCANmessage();
#endif

}

// ----------------------------------------------------------------------
void gui::on_pushButton_4_clicked() {
  fThread.setId(0x111);
  stringstream sbla; sbla << "CAN read ID = 0x" << hex << fThread.getId()
                          << " reg = 0x" << fThread.getRegister();
  
  fLOG(INFO, sbla.str());
#ifdef PI
  fThread.sendCANmessage();
#endif

}


// ----------------------------------------------------------------------
void gui::on_pushButton_5_clicked() {
  fThread.setId(0x101);
  stringstream sbla; sbla << "CAN send cmd ID = 0x" << hex << fThread.getId()
                          << " cmd = 0x" << fThread.getRegister();
  
  fLOG(INFO, sbla.str());
#ifdef PI
  fThread.sendCANmessage();
#endif

}


// ----------------------------------------------------------------------
#ifdef PI
void gui::on_toolButton_clicked() {
  stringstream sbla; sbla << "talk2FRAS";
  fLOG(INFO, sbla.str());
  fThread.talkToFras();
#endif

}


// ----------------------------------------------------------------------
void gui::on_spinBox_valueChanged(int arg1) {
  stringstream sbla; sbla << "spinBox_valueChanged to register = " << arg1;
  fLOG(ALL, sbla.str());
  fThread.setRegister(arg1);
}

// ----------------------------------------------------------------------
void gui::on_spinBox_2_valueChanged(int arg1) {
  stringstream sbla; sbla << "spinBox_valueChanged to value = " << arg1;
  fLOG(ALL, sbla.str());
  fThread.setValue(static_cast<float>(arg1));
}

// ----------------------------------------------------------------------
QString gui::getTimeString() {
    QDateTime dati = QDateTime::currentDateTime();
    int seconds = dati.time().second();
    int minutes = dati.time().minute();
    int hours   = dati.time().hour();
    int year    = dati.date().year();
    int month   = dati.date().month();
    int day     = dati.date().day();
    QString text;
    text = QString("%1/%2/%3 %4:%5:%6")
            .arg(year,4)
            .arg(month,2,10,QChar('0'))
            .arg(day,2,10,QChar('0'))
            .arg(hours,2,10,QChar('0'))
            .arg(minutes,2,10,QChar('0'))
            .arg(seconds,2,10,QChar('0'));
    return text;
}


// ----------------------------------------------------------------------
void gui::on_valueChanged(int v ) {
    //  if (fDoUpdate) {
    //    fDoUpdate = false;
    //    // -- cache values
    //    fXmin = fAxisX0->min();
    //    fXmax = fAxisX0->max();
    //    ui->graphicsView->chart()->scroll(v, 0);
    //    // fAxisX0->setMin(fXmin);
    //    // fAxisX0->setMax(fXmax - v);
    //  }
    //  // if (fAxisX0->max() > 0.9*fCurrent) {
    //  //   fDoUpdate = true;
    //  //   updateCPULoad();
    //  // }

    //  ui->graphicsView->chart()->scroll(v, 0);

}

// ----------------------------------------------------------------------
void gui::on_rangeChanged( qreal min, qreal max ) {

    //    if (fDoUpdate) {
    //      fDoUpdate = false;
    //      // -- cache values
    //      fAxisX0->setMin(min);
    //      fAxisX0->setMax(max);
    //      // fAxisX0->setMin(fXmin);
    //      // fAxisX0->setMax(fXmax - v);
    //    }
    //    ui->graphicsView->repaint();

}

void gui::on_graphUpdate(bool arg1) {
    fDoUpdate = arg1;
}
