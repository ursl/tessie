#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <string>
#include <cstdlib>

#include <fstream>
#include <iomanip>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include <fcntl.h>

#ifdef PI
#include <linux/i2c-dev.h>
#endif

// -- i2c address of SHT85 sensor
#define I2C_SHT85_ADDR 0x44

#include <chrono>

#include <thread>

#include "util.hh"
#include "tLog.hh"

using namespace std;

// ----------------------------------------------------------------------
driveHardware::driveHardware(tLog& x, int verbose): fLOG(x) {
  fRestart   = false;
  fAbort     = false;
  fRunCnt    = 0;
  fCANReg    = 0;
  fCANVal    = 0.;
  fVerbose   = verbose;
  QDateTime dt = QDateTime::currentDateTime();
  fDateAndTime = dt.date().toString() + "  " +  dt.time().toString("hh:mm");

  fCANReadIntVal = 3;
  fCANReadFloatVal = 3.1415;

  fI2CErrorCounter = 0;

  gettimeofday(&ftvStart, 0);
  fMilli5   = std::chrono::milliseconds(5);
  fMilli10  = std::chrono::milliseconds(10);
  fMilli100 = std::chrono::milliseconds(100);

  fCsvFileName = "tessie.csv";
  fLOG(INFO, stringstream("open" + fCsvFileName).str());
  fCsvFile.open(fCsvFileName, ios_base::app);

  // -- initialize all with 0
  for (unsigned int itec = 1; itec <= 8; ++itec) {
    fActiveTEC.insert(make_pair(itec, 0));
  }
  // -- turn on whatever
  for (unsigned int itec = 1; itec <= 8; ++itec) {
    fActiveTEC[itec] = 1;
  }

  fNActiveTEC = 0;
  for (unsigned int itec = 1; itec <= 8; ++itec) {
    if (1 == fActiveTEC[itec]) ++fNActiveTEC;
  }

  initTECData();

  fSHT85Temp = -99.;
  fSHT85RH   = -99.;
  fSHT85DP   = -99.;
  for (int i = 0; i < 6; ++i) {
    fSHT85Data[i] = 0;
  }

  fSHT85Config[0] = 0x24;   // MSB
  fSHT85Config[1] = 0x00;   // LSB

#ifdef PI

  // -- create I2C bus
  cout << "Open I2C bus for SHT85" << endl;

  char *bus = "/dev/i2c-0";
  if ((fSHT85File = open(bus, O_RDWR)) < 0) {
    cout << "Failed to open the bus." << endl;
    exit(1);
  } else {
    cout << "I2C bus opened with fSHT85File = " << fSHT85File << endl;
  }

  // -- read Sensirion SHT85 humidity/temperature sensor
  readSHT85();

  // -- eventually read the VProbe card(s)
  readVProbe();

#endif


#ifdef PI
  // -- write CAN socket
  memset(&fFrameW, 0, sizeof(struct can_frame));
  fSw = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fSw < 0) {
      perror("socket PF_CAN failed");
      return;
    }

  strcpy(fIfrW.ifr_name, "can0");
  int ret = ioctl(fSw, SIOCGIFINDEX, &fIfrW);
  if (ret < 0) {
      perror("ioctl failed");
      return;
    }
  
  fAddrW.can_family = AF_CAN;
  fAddrW.can_ifindex = fIfrW.ifr_ifindex;
  ret = bind(fSw, (struct sockaddr *)&fAddrW, sizeof(fAddrW));
  if (ret < 0) {
      perror("bind failed");
      return;
    }

  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  
  // -- read CAN socket
  memset(&fFrameR, 0, sizeof(struct can_frame));
  fSr = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fSr < 0) {
      perror("socket PF_CAN failed");
      return;
    }

  strcpy(fIfrR.ifr_name, "can0");
  ret = ioctl(fSr, SIOCGIFINDEX, &fIfrR);
  if (ret < 0) {
      perror("ioctl failed");
      return;
    }
  
  fAddrR.can_family = AF_CAN;
  fAddrR.can_ifindex = fIfrR.ifr_ifindex;
  ret = bind(fSr, (struct sockaddr *)&fAddrR, sizeof(fAddrR));
  if (ret < 0) {
      perror("bind failed");
      return;
    }

  //4.Define receive rules
  struct can_filter rfilter[1];
  rfilter[0].can_id = 0x000;
  rfilter[0].can_mask = CAN_SFF_MASK;
  // -- this does not work with the new CANBUS protocol?!
  // setsockopt(fSr, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));


  // -- add timeout?
  // https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
  // struct timeval tv;
  // tv.tv_sec = 0;
  // tv.tv_usec = 1000; // 1 millisecond
  // setsockopt(fSr, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  if (1) {
      // https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10; // 0.1 millisecond
      if (setsockopt(fSr /*rcv_sock*/, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0) {
        perror("Error in setting up time out");
      }
  }

#endif
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {

  fMutex.lock();
  fAbort = true;
  fCondition.wakeOne();
  fMutex.unlock();

#ifdef PI
  shutDown();
#endif
}


// ----------------------------------------------------------------------
void driveHardware::evtHandler() {
  // -- allow signals to reach slots
  QCoreApplication::processEvents();
  // -- make sure that no CAN errors are around
  parseCAN();
}


// ----------------------------------------------------------------------
void driveHardware::sentFromServer(QString msg) {
  fIoMessage = msg.toStdString();
  parseIoMessage();
}


// ----------------------------------------------------------------------
void driveHardware::doRun() {
  cout << "driveHardware::doRun() entered" << endl;
  int cnt(0);
  struct timeval tvVeryOld, tvOld, tvNew;
  gettimeofday(&tvVeryOld, 0);
  gettimeofday(&tvOld, 0);

  cout << "driveHardware::doRun() start loop" << endl;
  while (1) {
    evtHandler();

    ++cnt;
    std::this_thread::sleep_for(fMilli5);
    evtHandler();
    readCAN();
    gettimeofday(&tvNew, 0);
    int tdiff = diff_ms(tvNew, tvOld);
    int tdiff2 = diff_ms(tvNew, tvVeryOld);
    if (tdiff2 > 10000.) {
      tvVeryOld = tvNew;
      dumpMQTT(1);
    }
    if (tdiff > 1000.) {
      tvOld = tvNew;
      if (0) cout << tStamp() << " readAllParamsFromCANPublic(), tdiff = " << tdiff << endl;
      readSHT85();

      // -- read all parameters from CAN
      fMutex.lock();
      readAllParamsFromCANPublic();
      fMutex.unlock();

      evtHandler();

      // -- do something with the results
      if (0) cout << tStamp() << " emit signalUpdateHwDisplay tdiff = " << tdiff << endl;
      emit signalUpdateHwDisplay();
      dumpCSV();
      dumpMQTT();

      // -- make sure there is no alarm before clearing
      parseCAN();

      evtHandler();

      entertainFras();
      entertainTECs();

      ensureSafety();
    }
  }

}


// ----------------------------------------------------------------------
void driveHardware::ensureSafety() {
  // -- air temperatures
  if (fSHT85Temp > SAFETY_MAXSHT85TEMP) {
    stringstream a("==ALARM== Box air temperature = " +
                   to_string(fSHT85Temp) +
                   " exceeds SAFETY_MAXSHT85TEMP = " +
                   to_string(SAFETY_MAXSHT85TEMP));
    fLOG(ERROR, a.str());
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
    if (0) shutDown();
    emit signalSetBackground("T", "red");
  }

  // -- dew point vs air temperature
  if ((fSHT85Temp - SAFETY_DPMARGIN) < fSHT85DP) {
    stringstream a("==ALARM== Box air temperature = " +
                    to_string(fSHT85Temp) +
                    " is too close to dew point = " +
                    to_string(fSHT85DP));
    fLOG(ERROR, a.str());
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
    if (0) shutDown();
    emit signalSetBackground("DP", "red");
  }

  // -- check water temperature
  if (fTECData[8].reg["Temp_W"].value > SAFETY_MAXTEMPW) {
    stringstream a("==ALARM== Water temperature = " +
                    to_string(fTECData[8].reg["Temp_W"].value) +
                    " exceeds SAFETY_MAXTEMPW = " +
                    to_string(SAFETY_MAXTEMPW));
    fLOG(ERROR, a.str());
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
    if (0) shutDown();
  }

  // -- check module temperatures (1) value and (2) against dew point
  for (int itec = 1; itec <= 8; ++itec) {
    double mtemp = fTECData[itec].reg["Temp_M"].value;
    if (mtemp > SAFETY_MAXTEMPM) {
      stringstream a("==ALARM== module temperature = " +
                      to_string(mtemp) +
                      " exceeds SAFETY_MAXTEMPM = " +
                      to_string(SAFETY_MAXTEMPM));
      fLOG(ERROR, a.str());
      emit signalSendToMonitor(QString::fromStdString(a.str()));
      emit signalSendToServer(QString::fromStdString(a.str()));
      if (0) shutDown();
      QString qtec = QString::fromStdString("tec"+to_string(itec));
      emit signalSetBackground(qtec, "red");
    }

    if ((mtemp - SAFETY_DPMARGIN) < fSHT85DP) {
      stringstream a("==ALARM== module " + to_string(itec) + " temperature = " +
                     to_string(mtemp) +
                     " is too close to dew point = " +
                     to_string(fSHT85DP)
                    );
      fLOG(ERROR, a.str());
      emit signalSendToMonitor(QString::fromStdString(a.str()));
      emit signalSendToServer(QString::fromStdString(a.str()));
      if (0) shutDown();
      QString qtec = QString::fromStdString("tec"+to_string(itec));
      emit signalSetBackground(qtec, "red");
    }
  }
}

// ----------------------------------------------------------------------
void driveHardware::setRegister(int reg) {
  fCANReg = reg;
}

// ----------------------------------------------------------------------
void driveHardware::setValue(float val) {
  fCANVal = val;
}

// ----------------------------------------------------------------------
void driveHardware::setId(int val) {
  fCANId = val;
}


// ----------------------------------------------------------------------
int driveHardware::getRegister() {
  return fCANReg;
}

// ----------------------------------------------------------------------
float driveHardware::getValue() {
  return fCANVal;
}

// ----------------------------------------------------------------------
int driveHardware::getId() {
  return fCANId;
}

// ----------------------------------------------------------------------
int driveHardware::getTECRegisterIdx(std::string rname) {
  return fTECData[1].getIdx(rname);
}


// ----------------------------------------------------------------------
int driveHardware::getSWVersion(int itec) {
  int version(0);
  if (0 == fActiveTEC[itec]) {
    cout << "TEC " << itec <<  " not active, skipping" << endl;
    return version;
  }
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 6; // GetSWVersion
  fCANVal = 0; // nothing required
  sendCANmessage();
  std::this_thread::sleep_for(fMilli10);
  readCAN();

  canFrame a = fCanMsg.getFrame();
  stringstream sbla; sbla << "getSWVersion("
                          << itec << ")"
                          << " reg = " << fCANReg << hex
                          << " canID = 0x" << fCANId << dec
                          << " version = " << a.fIntVal;
  fLOG(INFO, sbla.str());


  return version;
}


// ----------------------------------------------------------------------
void  driveHardware::setTECParameter(float par) {
  fTECParameter = par;
  //  printf("driveHardware::setTECParameter = %f\n", fTECParameter);
  QString aline = QString("driveHardware::setTECParameter = ") + QString::number(par, 'f', 2);
  cout << aline.toStdString() << endl;

  emit signalText(aline);
}


// ----------------------------------------------------------------------
void driveHardware::shutDown() {
  // -- don't call this while things are warming up (from a previous shutDown call)
#ifdef PI
  for (int itec = 1; itec <= 8; ++itec) {
    turnOffTEC(itec);
    std::this_thread::sleep_for(fMilli5);
  }
  for (int itec = 1; itec <= 8; ++itec) {
    setTECRegister(itec, "ControlVoltage_Set", 0.0);
    std::this_thread::sleep_for(fMilli5);
  }

  std::this_thread::sleep_for(5*fMilli100);

  // -- wait 5 seconds
  for (int i = 0; i < 5; ++i) {
    // -- read all parameters from CAN
    fMutex.lock();
    readAllParamsFromCANPublic();
    fMutex.unlock();
    emit signalUpdateHwDisplay();
    std::this_thread::sleep_for(10*fMilli100);
  }

  // -- no?!
  //  close(fSw);
  //  close(fSr);
  //  close(fSHT85File);
#endif
}


// ----------------------------------------------------------------------
void driveHardware::readCAN(int nreads) {
  //fMutex.lock();
  int nbytes(0);

  bool DBX(false);
  // if (nreads > 1) DBX = true;

  static int cntCAN(0);

  char data[4] = {0, 0, 0, 0};

  unsigned int idata(0);
  float fdata(0.0);

  ++nreads;
  while (nreads > 0) {
    if (DBX) cout << "read(fSr, &fFrameR, sizeof(fFrameR)) ... nreads = " << nreads << endl;
#ifdef PI
    nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
#endif
    if (DBX) cout << "  nbytes = " << nbytes << endl;

    // -- this is an alternative to 'read()'
    //  socklen_t len = sizeof(fAddrR);
    //  nbytes = recvfrom(fSr, &fFrameR, sizeof(fFrameR), 0, (struct sockaddr*)&fAddrR, &len);

    if (nbytes > -1) {
      if (1) {
#ifdef PI
        canFrame f(fFrameR.can_id, fFrameR.can_dlc, fFrameR.data);
        fCanMsg.addFrame(f);
#endif
      }
    }
    --nreads;
  }
  parseCAN();
  //fMutex.unlock();
  return;
}


// ----------------------------------------------------------------------
void driveHardware::parseCAN() {
  if (fCanMsg.getFRASMessage() > 0) {
    entertainFras();
  }

  if (fCanMsg.getAlarm() > 0) {
    cout << "received alarm from CAN bus, do something!" << endl;
  }

  // -- print errors (if present) accumulated in CANmessage
  int nerrs = fCanMsg.nErrors();
  if (nerrs > 0) {
    fCANErrorOld = fCANErrorCounter;
    fCANErrorCounter = nerrs;
    deque<string> errs = fCanMsg.getErrors();
    int errRepeat(0);
    while (errs.size() > 0) {
      string errmsg = errs.front();
      if (string::npos != errmsg.find("parse issue")) {
          ++errRepeat;
      }
      if (errRepeat < 5) {
          fLOG(WARNING, errmsg);
      }
      errs.pop_front();
    }
    if (errRepeat > 5) {
      fLOG(WARNING, "truncated warning message " + to_string(errRepeat) + " times");
    }
  }
  fCanMsg.clearAllFrames();

}


// ----------------------------------------------------------------------
bool driveHardware::findInIoMessage(string &s1, string &s2, string &s3) {
  if (string::npos != fIoMessage.find(s3)) {
    if (string::npos != fIoMessage.find(s1)) return true;
    if (string::npos != fIoMessage.find(s2)) return true;
  }

  return false;
}


// ----------------------------------------------------------------------
void driveHardware::answerIoGet(string &) {
  string what = fIoMessage;

  cout << "answerIoSet what ->" << what << "<-" << endl;
  string delimiter(" ");

  string regname("nada");
  int tec(0);

  vector<string> tokens = split(what, ' ');
  for (unsigned int it = 0; it < tokens.size(); ++it) {
    if (string::npos != tokens[it].find("tec")) {
      ++it;
      tec = atoi(tokens[it].c_str());
    }
    if (string::npos != tokens[it].find("get")) {
      ++it;
      regname = tokens[it];
    }
  }

  stringstream str;
  str << regname << " = ";
  int ntec(1);
  for (int itec = 1; itec <= 8; ++itec) {
    if ((0 != tec) && (itec != tec)) continue;
    if (ntec > 1) str << ",";
    str << getTECRegister(itec, regname);
    ++ntec;
  }
  QString qmsg = QString::fromStdString(str.str());
  emit signalSendToServer(qmsg);
  return;
}


// ----------------------------------------------------------------------
void driveHardware::answerIoSet(string &) {
  string what = fIoMessage;

  cout << "answerIoSet what ->" << what << "<-" << endl;
  string delimiter(" ");

  string regname("nada");
  float value(-999.);
  int tec(0);

  vector<string> tokens = split(what, ' ');
  for (unsigned int it = 0; it < tokens.size(); ++it) {
    if (string::npos != tokens[it].find("tec")) {
      ++it;
      tec = atoi(tokens[it].c_str());
    }
    if (string::npos != tokens[it].find("set")) {
      ++it;
      regname = tokens[it];
      ++it;
      value = atof(tokens[it].c_str());
    }
  }

  if (value < -900.) {
    fLOG(WARNING, "no proper value: " + what );
  } else {
    cout << "register ->" << regname
         << "<- value ->" << value
         << "<- tec = " << tec
         << endl;
  }

  for (int itec = 1; itec <= 8; ++itec) {
    if ((0 != tec) && (itec != tec)) continue;
    setTECRegister(itec, regname, value);
  }
  return;
}


// ----------------------------------------------------------------------
void driveHardware::answerIoCmd() {
  string what = fIoMessage;

  cout << "answerIoCmd what ->" << what << "<-" << endl;
  string delimiter(" ");

  string cmdname("nada");
  int tec(0);

  vector<string> tokens = split(what, ' ');
  for (unsigned int it = 0; it < tokens.size(); ++it) {
    if (string::npos != tokens[it].find("tec")) {
      ++it;
      tec = atoi(tokens[it].c_str());
    }
    if (string::npos != tokens[it].find("cmd")) {
      ++it;
      cmdname = tokens[it];
    }
  }

  if (string::npos != cmdname.find("ClearError")) {
    fCANReg = 5;
  } else if (string::npos != cmdname.find("GetSWVersion")) {
    fCANReg = 6;
  } else if (string::npos != cmdname.find("SaveVariables")) {
    fCANReg = 7;
  } else if (string::npos != cmdname.find("LoadVariables")) {
    fCANReg = 8;
  } else if (string::npos != cmdname.find("Reboot")) {
    fCANReg = 255;
  } else if (string::npos != cmdname.find("Reset")) {
    fCANReg = 255;
  }

  stringstream str;
  str << cmdname << " = ";
  int ntec(1);
  for (int itec = 1; itec <= 8; ++itec) {
    if ((0 != tec) && (itec != tec)) continue;
    fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
    fCANVal = 0; // nothing required
    stringstream sbla; sbla << cmdname << " tec("
                            << itec << ")"
                            << " reg = " << fCANReg << hex
                            << " canID = 0x" << fCANId << dec;
    fLOG(INFO, sbla.str());
    sendCANmessage();
    std::this_thread::sleep_for(fMilli10);
    readCAN();
    canFrame a = fCanMsg.getFrame();
    if (5 == fCANReg) {
      if (ntec > 1) str << ",";
      str << itec;
      ++ntec;
    } else if (6 == fCANReg) {
      if (ntec > 1) str << ",";
      str << a.fIntVal;
      ++ntec;
    } else if (7 == fCANReg) {
      if (ntec > 1) str << ",";
      str << itec;
      ++ntec;
    } else if (8 == fCANReg) {
      if (ntec > 1) str << ",";
      str << itec;
      ++ntec;
    } else if (255 == fCANReg) {
      if (ntec > 1) str << ",";
      str << itec;
      ++ntec;
    }
  }
  QString qmsg = QString::fromStdString(str.str());
  emit signalSendToServer(qmsg);
  return;
}


// ----------------------------------------------------------------------
void driveHardware::parseIoMessage() { 
  string s1("Temp"), s2("Temperature"), s3("get"), s0("Temp_");
  // -- GET answers
  if (string::npos != fIoMessage.find("> ")) {

  } else if (string::npos != fIoMessage.find("get ")) {
    if (!findInIoMessage(s0, s0, s3) && findInIoMessage(s1, s2, s3)) {
      stringstream str;
      str << "Temp = " << getTemperature();
      QString qmsg = QString::fromStdString(str.str());
      emit signalSendToServer(qmsg);
      return;
    }

    s1 = string("RH");
    s2 = string("humidity");
    if (findInIoMessage(s1, s2, s3)) {
      stringstream str;
      str << "RH = " << getRH();
      QString qmsg = QString::fromStdString(str.str());
      emit signalSendToServer(qmsg);
      return;
    }

    s1 = string("DP");
    s2 = string("dew point");
    if (findInIoMessage(s1, s2, s3)) {
      stringstream str;
      str << "DP = " << getDP();
      QString qmsg = QString::fromStdString(str.str());
      emit signalSendToServer(qmsg);
      return;
    }

    s1 = "valve0"; s2 = "Valve0";
    if (findInIoMessage(s1, s2, s3)) {
      stringstream str;
      str << "valve0" << " = " << (getStatusValve0()?"on":"off");
      QString qmsg = QString::fromStdString(str.str());
      emit signalSendToServer(qmsg);
    }

    s1 = "valve1"; s2 = "Valve1";
    if (findInIoMessage(s1, s2, s3)) {
      stringstream str;
      str << "valve1" << " = " << (getStatusValve1()?"on":"off");
      QString qmsg = QString::fromStdString(str.str());
      emit signalSendToServer(qmsg);
    }

    s1 = "Mode";  s2 = "Mode";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Voltage";  s2 = "ControlVoltage_Set";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);

    s1 = "kp"; s2 = "PID_kp";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "ki"; s2 = "PID_ki";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "kd"; s2 = "PID_kd";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Temp_Set"; s2 = "Temp_Set";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Max"; s2 = "PID_Max";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Min"; s2 = "PID_Min";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);

    s1 = "Temp_W"; s2 = "Temp_W";   if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Temp_M"; s2 = "Temp_M";   if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Diff"; s2 = "Temp_Diff";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);

    s1 = "Peltier_U"; s2 = "Peltier_U";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Peltier_I"; s2 = "Peltier_I";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Peltier_P"; s2 = "Peltier_P";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Peltier_R"; s2 = "Peltier_R";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);

    s1 = "Supply_U"; s2 = "Supply_U";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Supply_I"; s2 = "Supply_I";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Supply_P"; s2 = "Supply_P";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);

    s1 = "Power"; s2 = "PowerState";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Error"; s2 = "Error";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);
    s1 = "Ref_U"; s2 = "Ref_U";  if (findInIoMessage(s1, s2, s3)) answerIoGet(s2);

  } else if (string::npos != fIoMessage.find("set ")) {
    s3 = "set ";

    s1 = "Mode";  s2 = "Mode";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "Voltage";  s2 = "ControlVoltage_Set"; if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "PowerState";  s2 = "Power"; if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);

    s1 = "PID_kp"; s2 = "kp";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "PID_ki"; s2 = "ki";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "PID_kd"; s2 = "kd";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);

    s1 = "Temp_Set"; s2 = "Temp_Set";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "Max"; s2 = "PID_Max";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "Min"; s2 = "PID_Min";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);
    s1 = "Ref_U"; s2 = "Ref_U";  if (findInIoMessage(s1, s2, s3)) answerIoSet(fIoMessage);

    s1 = "valve0"; s2 = "Valve0";
    if (findInIoMessage(s1, s2, s3)) {
      if (string::npos != fIoMessage.find("on")) {
        if (!getStatusValve0()) {
          toggleFras(1);
        }
      } else if (string::npos != fIoMessage.find("off")) {
        if (getStatusValve0()) {
          toggleFras(1);
        }
      }
    }

    s1 = "valve1"; s2 = "Valve1";
    if (findInIoMessage(s1, s2, s3)) {
      if (string::npos != fIoMessage.find("on")) {
        if (!getStatusValve1()) {
          toggleFras(2);
        }
      } else if (string::npos != fIoMessage.find("off")) {
        if (getStatusValve1()) {
          toggleFras(2);
        }
      }
    }

  } else if (string::npos != fIoMessage.find("cmd ")) {
    s3 = "cmd ";
    s1 = "Power_On";  s2 = "Power_On";
    if (findInIoMessage(s1, s2, s3)) {
      for (int itec = 1; itec <=8; ++itec) {
        turnOnTEC(itec);
      }
    }

    s1 = "Power_Off";  s2 = "Power_Off";
    if (findInIoMessage(s1, s2, s3)) {
      for (int itec = 1; itec <=8; ++itec) {
        turnOffTEC(itec);
      }
    }

    s1 = "valve0";  s2 = "valve0";
    if (findInIoMessage(s1, s2, s3)) {
      toggleFras(1);
    }

    s1 = "valve1";  s2 = "valve1";
    if (findInIoMessage(s1, s2, s3)) {
      toggleFras(2);
    }

    s1 = "ClearError"; s2 = "ClearError";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "GetSWVersion"; s2 = "Version";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "SaveVariables"; s2 = "Save";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "LoadVariables"; s2 = "Load";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "Reset"; s2 = "Reboot";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "quit";  s2 = "exit";
    if (findInIoMessage(s1, s2, s3)) {
      shutDown();
      stringstream sbla; sbla << "This is the end, my friend";
      fLOG(INFO, "This is the end, my friend -- tessie shutting down");
      exit(0);
    }
  } else if (string::npos != fIoMessage.find("help")) {

    vector<string> vhelp;
    vhelp.push_back("> ===================");
    vhelp.push_back("> hostname: coldbox01");
    vhelp.push_back("> thread:  ctrlTessie");
    vhelp.push_back("> ===================");
    vhelp.push_back("> ");
    vhelp.push_back("> Note: [tec {0|x}] can be before or after {get|set|cmd}");
    vhelp.push_back("> ");
    vhelp.push_back("> cmd messages:");
    vhelp.push_back("> -------------");
    vhelp.push_back("> cmd Power_On");
    vhelp.push_back("> cmd Power_Off");
    vhelp.push_back("> cmd valve0");
    vhelp.push_back("> cmd valve1");
    vhelp.push_back("> cmd [tec {0|x}] ClearError");
    vhelp.push_back("> cmd [tec {0|x}] GetSWVersion");
    vhelp.push_back("> cmd [tec {0|x}] SaveVariables");
    vhelp.push_back("> cmd [tec {0|x}] LoadVariables");
    vhelp.push_back("> cmd [tec {0|x}] Reboot");

    vhelp.push_back("> ");
    vhelp.push_back("> messages to write information:");
    vhelp.push_back("> ------------------------------");
    vhelp.push_back("> [tec {0|x}] set Mode {0,1}");
    vhelp.push_back("> [tec {0|x}] set ControlVoltage_Set 1.1");
    vhelp.push_back("> [tec {0|x}] set PID_kp 1.1");
    vhelp.push_back("> [tec {0|x}] set PID_ki 1.1");
    vhelp.push_back("> [tec {0|x}] set PID_kd 1.1");
    vhelp.push_back("> [tec {0|x}] set Temp_Set 1.1");
    vhelp.push_back("> [tec {0|x}] set PID_Max 1.1");
    vhelp.push_back("> [tec {0|x}] set PID_Min 1.1");
    vhelp.push_back("> set valve0 {on|off}");
    vhelp.push_back("> set valve1 {on|off}");

    vhelp.push_back("> ");
    vhelp.push_back("> messages to obtain information:");
    vhelp.push_back("> -------------------------------");
    vhelp.push_back("> get Temp");
    vhelp.push_back("> get RH");
    vhelp.push_back("> get DP");
    vhelp.push_back("> get valve0");
    vhelp.push_back("> get valve1");

    vhelp.push_back("> get [tec {0|x}] Mode");
    vhelp.push_back("> get [tec {0|x}] ControlVoltage_Set");
    vhelp.push_back("> get [tec {0|x}] PID_kp");
    vhelp.push_back("> get [tec {0|x}] PID_ki");
    vhelp.push_back("> get [tec {0|x}] PID_kd");
    vhelp.push_back("> get [tec {0|x}] Temp_Set");
    vhelp.push_back("> get [tec {0|x}] PID_Max");
    vhelp.push_back("> get [tec {0|x}] PID_Min");

    vhelp.push_back("> get [tec {0|x}] Temp_W");
    vhelp.push_back("> get [tec {0|x}] Temp_M");
    vhelp.push_back("> get [tec {0|x}] Temp_Diff");

    vhelp.push_back("> get [tec {0|x}] Peltier_U");
    vhelp.push_back("> get [tec {0|x}] Peltier_I");
    vhelp.push_back("> get [tec {0|x}] Peltier_R");
    vhelp.push_back("> get [tec {0|x}] Peltier_P");

    vhelp.push_back("> get [tec {0|x}] Supply_U");
    vhelp.push_back("> get [tec {0|x}] Supply_I");
    vhelp.push_back("> get [tec {0|x}] Supply_P");

    vhelp.push_back("> get [tec {0|x}] PowerState");
    vhelp.push_back("> get [tec {0|x}] Error");
    vhelp.push_back("> get [tec {0|x}] Ref_U");

    vhelp.push_back("> Tutorial for getting started:");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \" set valve0 on\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"set valve1 on\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"set ControlVoltage_Set 4.5\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"cmd Power_On\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"cmd Power_Off\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"set ControlVoltage_Set 0.0\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"set valve0 off\" ");
    vhelp.push_back("> mosquitto_pub -h coldbox01 -t \"ctrlTessie\" -m \"set valve1 off\" ");


    for (unsigned int i = 0; i < vhelp.size(); ++i) {
      emit signalSendToServer(QString::fromStdString(vhelp[i]));
    }
  }
  fIoMessage = "";
}


// ----------------------------------------------------------------------
// DECREPIT
void driveHardware::readCANmessage(/*DECREPIT*/) {
  fCANReadIntVal   += 1;
  fCANReadFloatVal += 0.1;
#ifdef PI
  int nbytes(0);

  // -- send read request
  sendCANmessage();

  bool DBX(false);
  int itec = 0;
  int ireg = 0;

  static int cntCAN(0);

  char data[4] = {0, 0, 0, 0};

  unsigned int idata(0);
  float fdata(0.0);

  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));

  // -- this is an alternative to 'read()'
  //  socklen_t len = sizeof(fAddrR);
  //  nbytes = recvfrom(fSr, &fFrameR, sizeof(fFrameR), 0, (struct sockaddr*)&fAddrR, &len);

  if (nbytes > -1) {

      if (DBX) {
          printf("can_id = 0x%X ncan_dlc = %d \n", fFrameR.can_id, fFrameR.can_dlc);
          int i = 0;
          cout << "data[] = ";
          if (DBX) for (i = 0; i < fFrameR.can_dlc; i++) {
              printf("%3d ", fFrameR.data[i]);
            }
        }

      itec    = fFrameR.can_id & 0xf;
      ireg    = fFrameR.data[0];
      data[0] = fFrameR.data[1];
      data[1] = fFrameR.data[2];
      data[2] = fFrameR.data[3];
      data[3] = fFrameR.data[4];

      if (DBX) cout << " ireg = " << ireg << " (fCANReg = " << fCANReg << ") ";

      memcpy(&fdata, data, sizeof fdata);
      memcpy(&idata, data, sizeof idata);
      if (DBX) {
          printf("float = %f/uint32 = %u", fdata, idata);
          ++cntCAN;
          printf(" (received CAN message %d)", cntCAN);
        }
      if (DBX) cout << endl;

      stringstream sbla; sbla << "CAN read canid = " << hex << fFrameR.can_id
                              << " tec = " << itec
                              << " reg = 0x"  << hex << ireg
                              << " value = " << fdata;
      if (DBX) cout << "sbla: " << sbla.str() << endl;
      fLOG(INFO, sbla.str());


      fCANReadIntVal = idata;
      fCANReadFloatVal = fdata;
    }
#endif

  return;
}


// ----------------------------------------------------------------------
void driveHardware::sendCANmessage() {
 //fMutex.lock();

#ifdef PI
  int itec = 0;
  itec = fCANId & 0xf;
  if (0) cout << "sendCANmessage() TEC " << itec << endl;

  if ((itec > 0) && (0 == fActiveTEC[itec])) {
      if (0) cout << "TEC " << itec <<  " not active, skipping" << endl;
      return;
  }

  char data[4] = {0, 0, 0, 0};
  fFrameW.can_id = fCANId;
  int dlength(0), command(0);
  if (0x0 == ((0x0f0 & fCANId)>>4)) {
      // -- x0x is command access and no subsequent 4 bytes are required.
      //    fCANReg indicates the command type
      dlength = 1;
      command = 1;
    }
  if (0x1 & ((0x0f0 & fCANId)>>4)) {
      // -- x1x is read access and no subsequent 4 bytes are required
      dlength = 1;
    }
  if (0x2 & ((0x0f0 & fCANId)>>4)) {
      // -- x2x is write access and the subsequent 4 bytes are the value to be written
      dlength = 5;
    }
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fCANReg;

  if (dlength > 1) {
      unsigned int intCanVal = 0;
      if (0 == fCANReg) {
        intCanVal = static_cast<unsigned int>(fCANVal);
        cout << "DBX interpreting as unsigned int ->" << intCanVal << "<-" << endl;
        memcpy(data, &intCanVal, sizeof intCanVal);
      } else {
        memcpy(data, &fCANVal, sizeof fCANVal);
      }
      fFrameW.data[1] = data[0];
      fFrameW.data[2] = data[1];
      fFrameW.data[3] = data[2];
      fFrameW.data[4] = data[3];
    }

  if (0) {
      if (1 == command) {
          cout << "   sendCANmessage: canid = " << fCANId << " cmd = " << fCANReg
               << endl;
        } else {
          cout << "   canid = " << fCANId << " reg = " << fCANReg
               << " value = " << fCANVal
               << " dlength = " << dlength
               << endl;

        }
      printf("    can_id  = 0x%X (from sendCANmessage())\n", fFrameW.can_id);
      printf("    can_dlc = %d\n", fFrameW.can_dlc);

      for (int i = 0; i < fFrameW.can_dlc; ++i) {
          printf("    data[%d] = %2x/%3d\r\n", i, fFrameW.data[i], fFrameW.data[i]);
        }
    }
  //6.Send message
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
  if (nbytes != sizeof(fFrameW)) {
    printf("CAN BUS Send Error frame[0]!\r\n");
  }

  // -- this is required to absorb the write request from fSr
  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));

#endif
  //fMutex.unlock();
}


// ----------------------------------------------------------------------
void driveHardware::entertainTECs() {
  fCANId = (CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 3; // Watchdog
  fCANVal = fTECParameter;
  sendCANmessage();
}

// ----------------------------------------------------------------------
void driveHardware::entertainFras() {
#ifdef PI
  if (0 == fValveMask) {
      fFrameW.can_id = CAN_RTR_FLAG | 0x41;
      int dlength(0);
      fFrameW.can_dlc = dlength;
      // stringstream sbla; sbla << "entertainFras "
      //                         << " reg = 0x"  << hex << fFrameW.can_id;
      // cout << "sbla: " << sbla.str() << endl;

      // -- Send message
      setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
      int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
      if (nbytes != sizeof(fFrameW)) {
        if (fVerbose > 0) printf("CAN BUS Send Error frame[0]!\r\n");
      }
    } else {
      fFrameW.can_id = 0x40;
      int dlength(1);
      fFrameW.can_dlc = dlength;
      fFrameW.data[0] = fValveMask;

      // -- Send message
      setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
      int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
      if (nbytes != sizeof(fFrameW)) {
        if (fVerbose > 0) printf("CAN BUS Send Error frame[0]!\r\n");
      }
    }
  // -- this is required to absorb the write request from fSr
  int nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
#endif
}


// ----------------------------------------------------------------------
void driveHardware::turnOnValve(int i) {
  if (0 == i) {
    if (!getStatusValve0()) toggleFras(1);
  }
  if (1 == i) {
    if (!getStatusValve1()) toggleFras(2);
  }
}


// ----------------------------------------------------------------------
void driveHardware::turnOffValve(int i) {
  if (0 == i) {
    if (getStatusValve0()) toggleFras(1);
  }
  if (1 == i) {
    if (getStatusValve1()) toggleFras(2);
  }
}


// ----------------------------------------------------------------------
void driveHardware::toggleFras(int imask) {
  int old = fValveMask;
  fValveMask = old xor imask;
  //  fMutex.lock();

  //            TEC:   ssP'..tt'aaaa
  //           FRAS:   0aa'aaaa'akkk
  //  fFrameW.can_id = 000'0100'0000 -> 0x040 for process
  //  fFrameW.can_id = 000'0100'0001 -> 0x041 for service
  //  fFrameW.can_id = 000'0100'0010 -> 0x042 for control

#ifdef PI
  unsigned int canid = 0x40;
  fFrameW.can_id = canid;
  int dlength(1);
  fFrameW.can_dlc = dlength;
  fFrameW.data[0] = fValveMask;
#endif
  stringstream sbla; sbla << "toggleFRAS(" << imask << ")";
  fLOG(INFO, sbla.str().c_str());

  // -- Send message
#ifdef PI
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
  if (nbytes != sizeof(fFrameW)) {
    printf("CAN BUS Send Error frame[0]!\r\n");
  }

  // -- this is required to absorb the write request from fSr
  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));

#endif
  //  fMutex.unlock();

}


// ----------------------------------------------------------------------
float driveHardware::getTECRegister(int itec, std::string regname) {
  if (fTECData.find(itec) == fTECData.end()) {
      return -1.;
    }
  if (fTECData[itec].reg.find(regname) != fTECData[itec].reg.end()) {
      return fTECData[itec].reg[regname].value;
    } else {
      return -3.;
    }
}


// ----------------------------------------------------------------------
void  driveHardware::turnOnTEC(int itec) {
  if (0 == fActiveTEC[itec]) {
    cout << "TEC " << itec <<  " not active, skipping" << endl;
    return;
  }
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 1; // Power_On
  fCANVal = fTECParameter;
  stringstream sbla; sbla << "turnOnTEC("
                          << itec << ")"
                          << " reg = " << fCANReg << hex
                          << " canID = 0x" << fCANId << dec;
  fLOG(INFO, sbla.str());
  sendCANmessage();
}


// ----------------------------------------------------------------------
void  driveHardware::turnOffTEC(int itec) {
  if (0 == fActiveTEC[itec]) {
    cout << "TEC " << itec <<  " not active, skipping" << endl;
    return;
  }
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 2; // Power_Off
  fCANVal = fTECParameter;
  stringstream sbla; sbla << "turnOffTEC("
                          << itec << ")"
                          << " reg = " << fCANReg << hex
                          << " canID = 0x" << fCANId << dec;
  fLOG(INFO, sbla.str());
  sendCANmessage();
}


// ----------------------------------------------------------------------
float driveHardware::getTECRegisterFromCAN(int itec, std::string regname) {
  if (0) cout << "getTECRegisterFromCAN regname ->" << regname << "<-" << endl;
  if (itec > 0) {
    if (0 == fActiveTEC[itec]) {
      return -99.;
    }
  }

  if (itec > 0) {
    fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_READ);
    fCANReg = fTECData[itec].getIdx(regname);
  } else {
    fCANId = (0 | CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_READ);
    fCANReg = fTECData[1].getIdx(regname);
  }
  // -- send read request
  sendCANmessage();
  std::this_thread::sleep_for(fMilli10);
  if (0) cout << "  getTECRegisterFromCAN for tec = " << itec
              << " register = "<< regname
              << " regidx = " << fCANReg
              << endl;

  if (itec > 0) {
    readCAN();
    fCANReadFloatVal = fCanMsg.getFloat(itec, fCANReg);
    return fCANReadFloatVal;
  } else {
    readCAN(fNActiveTEC);
    return -97;
  }
  return -96;
}


// ----------------------------------------------------------------------
void driveHardware::setTECRegister(int itec, std::string regname, float value) {
  fTECData[itec].reg[regname].value = value;

  // -- program parameter
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_WRITE);

  fCANReg = fTECData[itec].getIdx(regname);

  fCANVal = value;

  QString aline = QString("write TEC register ") + QString::number(fCANReg, 'd', 0)
      + QString(" value =  ") + QString::number(fCANVal, 'f', 2)
      + QString(", canID = 0x") + QString::number(fCANId, 'x', 0)
      ;

  fLOG(INFO, aline.toStdString().c_str());

  sendCANmessage();
}


// ----------------------------------------------------------------------
void driveHardware::initTECData() {
  for (unsigned int itec = 1; itec <=8; ++itec) {
    fTECData.insert(make_pair(itec, initAllTECRegister()));
  }
}


// ----------------------------------------------------------------------
TECData  driveHardware::initAllTECRegister() {
  TECData tdata;

  TECRegister b;
  // -- read/write registers
  b = {1,      "Mode",                 0, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {0.,     "ControlVoltage_Set",   1, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {1.,     "PID_kp",               2, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {2.,     "PID_ki",               3, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {3.,     "PID_kd",               4, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {23.1,   "Temp_Set",             5, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "PID_Max",              6, 1}; tdata.reg.insert(make_pair(b.name, b));
  b = {-1.,    "PID_Min",              7, 1}; tdata.reg.insert(make_pair(b.name, b));

  // -- read-only registers
  b = {-99.,   "Temp_W",               8, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Temp_M",               9, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Temp_Diff",           10, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Peltier_U",           11, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Peltier_I",           12, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Peltier_R",           13, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Peltier_P",           14, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Supply_U",            15, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Supply_I",            16, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Supply_P",            17, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "PowerState",          18, 2}; tdata.reg.insert(make_pair(b.name, b));
  b = {0,      "Error",               19, 2}; tdata.reg.insert(make_pair(b.name, b));

  // -- anothe read/write register
  b = {-99.,   "Ref_U",               20, 1}; tdata.reg.insert(make_pair(b.name, b));

  // -- commands
  b = {-99.,   "NoCommand",            0, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Power_On",             1, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Power_Off",            2, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Watchdog",             3, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "Alarm",                4, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "ClearError",           5, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "GetSWVersion",         6, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "SaveVariables",        7, 3}; tdata.reg.insert(make_pair(b.name, b));
  b = {-99.,   "LoadVariables",        8, 3}; tdata.reg.insert(make_pair(b.name, b));

  b = {-99.,   "Reboot/Reset",       255, 3}; tdata.reg.insert(make_pair(b.name, b));

  return tdata;
}


// ----------------------------------------------------------------------
void driveHardware::readAllParamsFromCANPublic() {
  ++fRunCnt;
  // -- what to read: float
  vector<string> regnames = {"ControlVoltage_Set"
                            , "PID_kp"
                            , "PID_ki"
                            , "PID_kd"
                            , "Temp_Set"
                            , "PID_Max"
                            , "PID_Min"
                            , "Temp_W"
                            , "Temp_M"
                            , "Temp_Diff"
                            , "Peltier_U"
                            , "Peltier_I"
                            , "Peltier_R"
                            , "Peltier_P"
                            , "Supply_U"
                            , "Supply_I"
                            , "Supply_P"
                            , "PowerState"
                            , "Error"
                            , "Ref_U"

                            };
  for (unsigned int ireg = 0; ireg < regnames.size(); ++ireg) {
    if (0 == ireg%2) evtHandler();
    // -- NOTE: ireg != regnumber
    if (7 == ireg) {
      fTECData[8].reg["Temp_W"].value = getTECRegisterFromCAN(8, regnames[ireg]);
    } else if (9 == ireg) {
      fTECData[8].reg["Temp_Diff"].value = getTECRegisterFromCAN(8, regnames[ireg]);
    } else {
      getTECRegisterFromCAN(0, regnames[ireg]);
      if (0) cout << "  " << tStamp() << " reading broadcast "<< regnames[ireg] << endl;
      int regIdx = fTECData[1].getIdx(regnames[ireg]);
      for (int i = 1; i <= 8; ++i) {
        if (0 == fActiveTEC[i]) continue;
        fTECData[i].reg[regnames[ireg]].value = fCanMsg.getFloat(i, regIdx);
      }
    }
  }

  evtHandler();

  // -- read integer Mode
  getTECRegisterFromCAN(0, "Mode");
  int regIdx = fTECData[1].getIdx("Mode");
  for (int i = 1; i <= 8; ++i) {
    if (0 == fActiveTEC[i]) continue;
    fTECData[i].reg["Mode"].value = fCanMsg.getInt(i, regIdx);
  }

  // -- read integer PowerState
  getTECRegisterFromCAN(0, "PowerState");
  regIdx = fTECData[1].getIdx("PowerState");
  for (int i = 1; i <= 8; ++i) {
    if (0 == fActiveTEC[i]) continue;
    fTECData[i].reg["PowerState"].value = fCanMsg.getInt(i, regIdx);
  }

  // -- read integer Error
  getTECRegisterFromCAN(0, "Error");
  regIdx = fTECData[1].getIdx("Error");
  for (int i = 1; i <= 8; ++i) {
    if (0 == fActiveTEC[i]) continue;
    fTECData[i].reg["Error"].value = fCanMsg.getInt(i, regIdx);
  }

}


// ----------------------------------------------------------------------
void driveHardware::dumpMQTT(int all) {
  static map<int, TECData> oldTECData;

  // -- what to read: float
  map<string, double> tolerances = {
      {"ControlVoltage_Set", 0.1}
      , {"PID_kp", 0.1}
      , {"PID_ki", 0.1}
      , {"PID_kd", 0.1}
      , {"Temp_Set", 0.1}
      , {"PID_Max", 0.1}
      , {"PID_Min", 0.1}
      , {"Temp_W", 0.1}
      , {"Temp_M", 0.1}
      , {"Temp_Diff", 0.1}
      , {"Peltier_U", 0.1}
      , {"Peltier_I", 0.1}
      , {"Peltier_R", 0.1}
      , {"Peltier_P", 0.1}
      , {"Supply_U", 0.1}
      , {"Supply_I", 1.0}
      , {"Supply_P", 1.0}
      , {"PowerState", 0.1}
      , {"Error", 0.5}
      , {"Ref_U", 0.1}
      , {"Mode", 0.1}
  };

  for (auto const &skey: tolerances) {
    stringstream ss;
    ss << skey.first << " = ";
    bool printit(false);
    for (int i = 1; i <= 8; ++i) {
      ss << fTECData[i].reg[skey.first].value;
      if (1 == all) {
        printit = true;
      } else {
        if (fabs(fTECData[i].reg[skey.first].value - oldTECData[i].reg[skey.first].value) > tolerances[skey.first]) {
          printit = true;
        }
      }
      if (i < 8) ss << ",";
    }
    if (printit)  emit signalSendToMonitor(QString::fromStdString(ss.str()));
  }
  oldTECData = fTECData;
}


// ----------------------------------------------------------------------
void driveHardware::dumpCSV() {
  stringstream output;
  char cs[100];
  sprintf(cs, "%+5.2f,%05.2f,%+5.2f", fSHT85Temp, fSHT85RH, fSHT85DP);
  output << timeStamp() << "," << cs;

  // -- only one water temperature reading
  for (int i = 8; i <= 8; ++i) {
    sprintf(cs, "%+4.1f", fTECData[i].reg["Temp_W"].value);
    if (fActiveTEC[i]) output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    sprintf(cs, "%1.0f", fTECData[i].reg["PowerState"].value);
    if (fActiveTEC[i]) output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    sprintf(cs, "%+5.2f", fTECData[i].reg["ControlVoltage_Set"].value);
    if (fActiveTEC[i]) output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    sprintf(cs, "%+4.1f", fTECData[i].reg["Temp_Set"].value);
    if (fActiveTEC[i]) output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    sprintf(cs, "%+5.2f", fTECData[i].reg["Temp_M"].value);
    if (fActiveTEC[i]) output << "," << cs;
  }

  fCsvFile <<  output.str() << endl;
}

// ----------------------------------------------------------------------
string driveHardware::timeStamp(bool filestamp) {
  return fLOG.timeStamp(filestamp);
}

// ----------------------------------------------------------------------
void driveHardware::readSHT85() {
#ifdef PI
  // -- set SHT85 I2C address
  ioctl(fSHT85File, I2C_SLAVE, I2C_SHT85_ADDR);

  // -- send high repeatability measurement command
  //    command msb, command lsb(0x2C, 0x06)
  write(fSHT85File, fSHT85Config, 2);
  std::this_thread::sleep_for(fMilli100);

  // -- read 6 bytes of data
  //    temp msb, temp lsb, temp CRC, humidity msb, humidity lsb, humidity CRC
  if (read(fSHT85File, fSHT85Data, 6) != 6) {
    fLOG(WARNING, "I2C Error: Input/output Error with SHT85");
    fI2CErrorOld = fI2CErrorCounter;
    ++fI2CErrorCounter;
  } else {
    // -- convert the data
    //double cTemp = (((fSHT85Data[0] * 256) + fSHT85Data[1]) * 175.0) / 65535.0  - 45.0;
    //double humidity = (((fSHT85Data[3] * 256) + fSHT85Data[4])) * 100.0 / 65535.0;
    double norm = 65535.0;
    double st   = (fSHT85Data[0]<<8) + fSHT85Data[1];
    fSHT85Temp  = (st * 175.0) / norm  - 45.0;

    st          = (fSHT85Data[3]<<8) + fSHT85Data[4];
    fSHT85RH    = (st * 100.0) / norm;

    fSHT85DP = calcDP(1);

    // -- print
    if (0) {
      cout << "Temperature in Celsius: " << fSHT85Temp << endl;
      cout << "Relative Humidity:      " << fSHT85RH << endl;
    }
  }
#endif
}

// ----------------------------------------------------------------------
void driveHardware::readVProbe() {
  return;
#ifdef PI
    // TODO FIXME implement this once something is installed
    int length;
    unsigned char buffer[60] = {0};

    // -- FIXME set I2C address
    ioctl(fSHT85File, I2C_SLAVE, 0x3e);


    length = 16;			//<<< Number of bytes to read
    // read() returns the number of bytes actually read, if it doesn't match
    // then an error occurred (e.g. no response from the device)
    if (read(fSHT85File, buffer, length) != length) {
        //ERROR HANDLING: i2c transaction failed
        printf("Failed to read from the i2c bus.\n");
    } else {
        printf("Data read: %s\n", buffer);
    }

#endif
}

// ----------------------------------------------------------------------
float driveHardware::getTemperature() {
  return fSHT85Temp;
}

// ----------------------------------------------------------------------
float driveHardware::getRH() {
  return fSHT85RH;
}

// ----------------------------------------------------------------------
float driveHardware::getDP() {
  return fSHT85DP;
}

// ----------------------------------------------------------------------
int driveHardware::getRunTime() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  int dt = tv.tv_sec - ftvStart.tv_sec;
  return dt;
}

// ----------------------------------------------------------------------
int driveHardware::diff_ms(timeval t1, timeval t2) {
  return (((t1.tv_sec - t2.tv_sec) * 1000000) +
          (t1.tv_usec - t2.tv_usec))/1000;
}


// ----------------------------------------------------------------------
// -- calculate dew point
//    https://doi.org/10.1175/BAMS-86-2-225
float driveHardware::calcDP(int mode) {
  double dp(0.);
  if (0 == mode) {
    dp = fSHT85Temp - (100. - fSHT85RH)/5.; // most simple approximation
  } else if (1 == mode) {
    double A1(17.625), B1(243.04);
    double td0 = B1 * (log(fSHT85RH/100.) + (A1*fSHT85Temp)/(B1 + fSHT85Temp));
    double td1 = A1 - log(fSHT85RH/100.) - (A1*fSHT85Temp)/(B1 + fSHT85Temp);
    dp = td0/td1;

  }

  return static_cast<float>(dp);
}
