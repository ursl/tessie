#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <QtCore/QObject>

#include "tMosq.hh"

class driveHardware;
// ----------------------------------------------------------------------
class ioServer: public QObject {

  Q_OBJECT

public:
  ioServer(driveHardware *);
  void printFromServer(std::string result);
  virtual ~ioServer();

public slots:
  void sentToServer(const QString &result);
  void startServer();
  void run();

signals:
  void sendFromServer(const QString &result);

private:
  tMosq         *fCtrlTessie;
  driveHardware *fHardware;
};

#endif // RPCSERVER_H
