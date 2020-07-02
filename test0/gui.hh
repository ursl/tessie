#ifndef GUI_H
#define GUI_H

#include <QtWidgets/QMainWindow>

#include "driveHardware.hh"

class gui : public QMainWindow {
  Q_OBJECT

public:
  gui(QMainWindow *parent = nullptr);
  ~gui();


protected:
  void paintEvent(QPaintEvent *event) override;


private slots:
  void updateFrequency(int f);

private:
  driveHardware fThread;
  int fInputFrequency, fInputOffset;

};

#endif // GUI_H
