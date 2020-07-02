#ifndef DIALOG_H
#define DIALOG_H

#include <QtWidgets/QWidget>

#include "driveHardware.hh"

class dialog : public QWidget {
  Q_OBJECT

public:
  dialog(QWidget *parent = nullptr);
  ~dialog();


protected:
  void paintEvent(QPaintEvent *event) override;


private slots:
  void updateFrequency(int f);

private:
  driveHardware fThread;
  int fInputFrequency, fInputOffset;

};

#endif // DIALOG_H
