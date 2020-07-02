#ifndef GUI_H
#define GUI_H

#include <QtWidgets/QMainWindow>

#include "driveHardware.hh"
#include "ui_gui.h"

class gui : public QMainWindow {
  Q_OBJECT

public:
  gui(QMainWindow *parent = nullptr);
  ~gui();


protected:
  void paintEvent(QPaintEvent *event) override;


private slots:
  void updateFrequency(int f);

  void on_pushButton_clicked();

  void on_pushButton_2_clicked();

  void on_spinBox_valueChanged(int arg1);

  void on_spinBox_2_valueChanged(int arg1);

private:
  driveHardware fThread;
  int fInputFrequency, fInputOffset;

  // -- without the following line you cannot 'go to slot' in the UI designer (and this line requires the above include)
  Ui::MainWindow *ui;

};

#endif // GUI_H
