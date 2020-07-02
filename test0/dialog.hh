#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
  class Dialog;
}

class dialog : public QDialog {
  Q_OBJECT

public:
  explicit dialog(QWidget *parent = nullptr);
  ~dialog();

private:
  Ui::Dialog *ui;
};

#endif // DIALOG_H
