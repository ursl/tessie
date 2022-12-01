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
  void printFromServer(const QString &result);
  virtual ~ioServer();

public slots:
  void sentToServer(const QString &result);
  void startServer();
  void run();

signals:
  void sendFromServer(const QString &result);

private:
  tMosq   *fMosq;

  QString fString;
  float   fOldTemperature;
};

#endif // RPCSERVER_H
