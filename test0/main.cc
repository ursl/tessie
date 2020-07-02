
#include <QtWidgets/QApplication>
#include "dialog.hh"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication app(argc, argv);
  dialog widget;
  widget.show();
  return app.exec();
}
