#ifndef TECDISPLAY_H
#define TECDISPLAY_H

#include <QWidget>

namespace Ui {
  class TECDisplay;
}

class TECDisplay : public QWidget {
  Q_OBJECT

public:
  explicit TECDisplay(QWidget *parent = nullptr);
  ~TECDisplay();

private:
  Ui::TECDisplay *ui;
};

#endif // TECDISPLAY_H
