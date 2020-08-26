#ifndef GUI_H
#define GUI_H

#include <QtWidgets/QMainWindow>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>


#include "driveHardware.hh"
#include "tLog.hh"
#include "ui_gui.h"

class gui : public QMainWindow {
  Q_OBJECT

public:
  gui(tLog &x, QMainWindow *parent = nullptr);
  void printText(std::string line);
  ~gui();


protected:


private slots:
  void appendText(QString line);

  void on_pushButton_clicked();

  void on_pushButton_2_clicked();

  void on_spinBox_valueChanged(int arg1);

  void on_spinBox_2_valueChanged(int arg1);

  QString getTimeString();

  void updateTime();

#ifdef PI
  void on_toolButton_clicked(bool checked);
#endif

private:
  tLog&         fLOG;
  driveHardware fThread;
  int fInputFrequency, fInputOffset;

  QtCharts::QLineSeries *fSeries;
  QtCharts::QChart      *fChart;

  // -- without the following line you cannot 'go to slot' in the UI designer (and this line requires the above include)
  Ui::MainWindow *ui;

};

#endif // GUI_H
