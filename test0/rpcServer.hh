#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <QtCore/QObject>

class rpcServer: public QObject {

  Q_OBJECT

public:
  rpcServer();
  virtual ~rpcServer();

public slots:
  void sentToServer(const QString &result);
  void run();

signals:
  void sendFromServer(const QString &result);

private:
  QString fString;
};

#endif // RPCSERVER_H
