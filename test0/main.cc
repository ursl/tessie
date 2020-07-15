#include <iostream>
#include <QtWidgets/QApplication>

#include "gui.hh"

using namespace std;

int main(int argc, char *argv[]) {
  // -- by default run in full screen mode
  bool doRunInWindow(false);

  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-h")) {
      cout << "List of arguments:" << endl;
      cout << "-w                    start in window (not fullscreen)" << endl;
      return 0;
    }
    if (!strcmp(argv[i], "-w")) {doRunInWindow = true;}
  }


  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication app(argc, argv);

  gui theGui;
  if (!doRunInWindow) {
    theGui.showFullScreen();
  }

  return app.exec();
}
