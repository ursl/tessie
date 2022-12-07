#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <QtCore/QObject>

#include "tMosq.hh"

class driveHardware;
// ----------------------------------------------------------------------
class ioServer: public QObject {
  Q_OBJECT

public:
  ioServer();
  ~ioServer();
  void printFromServer(QString msg);
  void start() {startServer(); }
  void startServer();

public slots:
  void run();
  void sentToServer(QString msg);

signals:
  void sendFromServer(QString msg);

private:
  tMosq         *fCtrlTessie;
};

#endif // RPCSERVER_H
