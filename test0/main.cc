
#include <QtWidgets/QApplication>
#include "ui_gui.h"
#include "gui.hh"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication app(argc, argv);
  // -- here you connect the ui_gui (QML design) with the GUI class:
  gui theGui;
  Ui_MainWindow ui;
  ui.setupUi(&theGui);
  theGui.show();

  return app.exec();
}
