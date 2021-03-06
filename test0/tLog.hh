#ifndef LOG_H
#define LOG_H

#include <QtCore/QtCore>

#include <fstream>
#include <string>
#include <cstdio>
#include <iomanip>
#include <sys/time.h>

inline std::string NowTime();

class gui;
class driveHardware;

enum tLogLevel {ERROR, WARNING, INFO, DEBUG, ALL};

class tLog: public QObject {
  Q_OBJECT
public:
  tLog(std::string fname = "tessie.txt");
  virtual ~tLog();
public:
  void setLevel(std::string);
  tLogLevel getLevel();
  void operator()(tLogLevel, std::string);
  // -- same as above but also returns the string produced
  std::string print(tLogLevel, std::string);

  static std::string toString(tLogLevel level);
  static tLogLevel fromString(const std::string& level);
  std::string timeStamp(bool filestamp = false);
  void setGui(gui *x) {fpGui = x;}
  void setHw(driveHardware *x) {fpHw = x;}
signals:
  void signalText(QString x);

private:
  tLog(const tLog&);
  tLog& operator = (const tLog&);

  gui *fpGui;
  driveHardware *fpHw;
  tLogLevel fLevel;
  std::string fFileName;
  std::ofstream fFile;
};

#endif
