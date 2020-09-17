#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <QtCore/QObject>

class driveHardware;
// ----------------------------------------------------------------------
class rpcServer: public QObject {

  Q_OBJECT

public:
  rpcServer(driveHardware *);
  void printFromServer(const QString &result);
  virtual ~rpcServer();

public slots:
  void sentToServer(const QString &result);
  void run();

signals:
  void sendFromServer(const QString &result);

private:
  QString fString;
  float   fOldTemperature;
};

#endif // RPCSERVER_H
