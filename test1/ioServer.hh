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
  void startServer();

public slots:
  void run();
  void sentToServer(QString msg);

signals:
  void signalSendFromServer(QString msg);

private:
  tMosq         *fCtrlTessie;
};

#endif // RPCSERVER_H
