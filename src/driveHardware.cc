#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#include <fstream>
#include <iomanip>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/statvfs.h>

#include "rsstools.hh"

#include <fcntl.h>

#include "tec_errors.hh"

#ifdef PI
#include <pigpiod_if2.h>
#endif

#ifdef UZH
#define I2C_PCA_ADDR   0x41
#endif

// -- i2c address of SHT85 sensor
#define I2C_SHT85_ADDR 0x44

// -- i2c addresses of HYT-223 sensor and related GPIO expander
#define I2C_HYT223_ADDR 0x28
#define I2C_HEATHYT223_ADDR 0x4F


// -- i2c address of flowmeter
#define I2C_FLOWMETER_ADDR 0x41

// -- define GPIO pins for side lights and INTL/PSEN
//     physical 11/13/15 LED pins  GPIO 17/27/22
//     physical 16       INTL      GPIO 23
//     physical 18       PSEN      GPIO 24    // this is 3.3V enable for central PCB

#define GPIORED   17
#define GPIOYELLO 27
#define GPIOGREEN 22
#define GPIOPSUEN 24
#define GPIOINT   23

#define I2CBUS    0

#define HEATER_MAX_STATUS 180

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
  //fVerbose   = 6;
  QDateTime dt = QDateTime::currentDateTime();
  fDateAndTime = dt.date().toString() + "  " +  dt.time().toString("hh:mm");

  fCANReadIntVal = 3;
  fCANReadFloatVal = 3.1415;

  fI2CErrorCounter = 0;
  fI2CErrorOld = 0;
  fStopOperations = 0;
  // -- negative means not yet set, 0 is no flow, 1 is flow enable
  fFlowMeterStatus = -1; 
  fBadFlowMeterReading = 0;
  fThrottleStatus = 0;
  fHeaterStatus = 0; 
  fReconditioning = 0;
  fReconditioningWaitTime = 0;

  fTrafficRed = fTrafficYellow = fTrafficGreen = 0; 
  
  gettimeofday(&ftvStart, 0);
  fMilli5   = std::chrono::milliseconds(5);
  fMilli10  = std::chrono::milliseconds(10);
  fMilli20  = std::chrono::milliseconds(20);
  fMilli100 = std::chrono::milliseconds(100);
  fMilli500 = std::chrono::milliseconds(500);

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
  for (int itec = 1; itec <= 8; ++itec) {
    fTECTurnedOn[itec] = false;
  }

  fSHT85Temp = -99.;
  fSHT85RH   = -99.;
  fSHT85DP   = -999.;
  for (int i = 0; i < 6; ++i) {
    fSHT85Data[i] = 0;
  }

  fHYT223Temp = -99.;
  fHYT223RH   = -99.;
  fHYT223DP   = -999.;
  for (int i = 0; i < 4; ++i) {
    fHYT223Data[i] = 0;
  }

  fAirTemp = -99.;
  fAirRH   = -99.;
  fAirDP   = -999.;
  
  fSHT85Config[0] = 0x24;   // MSB
  fSHT85Config[1] = 0x00;   // LSB

  fRelaisMask      = 0;
  fAlarmState      = 0;
  fLidStatus       = 0;
  fInterlockStatus = 0;
  fLidReading      = -99999.;
  
#ifdef PI
  fSw = -1;
  fSr = -1;
  fPiGPIO = pigpio_start(NULL, NULL);

  cout << "pigpio_start() = " << fPiGPIO << endl;

  set_mode(fPiGPIO, GPIORED,  PI_OUTPUT);
  set_mode(fPiGPIO, GPIOGREEN, PI_OUTPUT);
  set_mode(fPiGPIO, GPIOYELLO, PI_OUTPUT);

  set_mode(fPiGPIO, GPIOPSUEN, PI_OUTPUT);
  gpio_write(fPiGPIO, GPIOPSUEN, 1);

  set_mode(fPiGPIO, GPIOINT, PI_OUTPUT);
  // -- start with interlock high (will go to low in first call to ensureSafety)
  gpio_write(fPiGPIO, GPIOINT, 1);
  fInterlockStatus = 1;
  
  lighting(1);

  // -- read Sensirion SHT85 humidity/temperature sensor
  cout << "initial readout air temperature" << endl;
  readAirTemperature();

  if (!initCANSockets()) return;

  fOldLidStatus = 2; // initial value
  checkLid();

  // -- Load TEC parameters from FLASH
  loadFromFlash();
  std::this_thread::sleep_for(fMilli100);


  // -- read firmware version (have it printed) and make sure that all TECs have the same version
  int version1 = getSWVersion(1);
  fLOG(INFO, "TEC 1 firmware version = " + to_string(version1));

  int version(-1);
  bool versionOK(true);
  for (int i = 2; i <= 8; ++i) {
    version = getSWVersion(i);
    fLOG(INFO, "TEC " + to_string(i) + " firmware version = " + to_string(version));
    if (version != version1) {
      versionOK = false;
    }
  }
  if (!versionOK) {  
    fStatusString = "TEC firmware mismatch";
  } else {
    if (version < 12) {
      fLOG(INFO, "Upgrade TEC f/w! Current version = " + to_string(version));
      fStatusString = "TEC f/w < 12";
      versionOK = false;
    }
  }

  fVersionOK = versionOK;

  fStatusString = "initialization OK";


#endif

  char hostname[1024];
  gethostname(hostname, 1024);
  fHostName = hostname;

#ifdef UZH
  fBusyDisarm = false;
#endif


#ifdef PI
  // -- Check I2C addresses
  fLOG(INFO, "Check I2C slaves"); 
  char data[6];
  
  std::this_thread::sleep_for(fMilli100);
  std::this_thread::sleep_for(fMilli100);
  vector<unsigned int> vi2c = {I2C_SHT85_ADDR
                               , I2C_HYT223_ADDR
                               , I2C_HEATHYT223_ADDR
                               , I2C_FLOWMETER_ADDR
  };
  int handle(0), length(-1);

  // -- essential?!
  fI2CSlaveStatus.clear();

  for (auto it: vi2c) {
    handle = length = -1;
    if (it == I2C_SHT85_ADDR) {
      // -- SHT895 needs command written first
      handle = i2c_open(fPiGPIO, I2CBUS, it, 0);
      int result = i2c_write_device(fPiGPIO, handle, fSHT85Config, 2);
      std::this_thread::sleep_for(fMilli20);
      length = i2c_read_device(fPiGPIO, handle, data, 6);
    } else {
      // -- rest responds with simple read
      handle = i2c_open(fPiGPIO, I2CBUS, it, 0);
      length = i2c_read_device(fPiGPIO, handle, data, 4);
    }
    i2c_close(fPiGPIO, handle);
    
    if (length > 0) {
      fI2CSlaveStatus.insert({it, true});
    } else {
      fI2CSlaveStatus.insert({it, false});
    }
  }

  for (auto it: fI2CSlaveStatus) {
    stringstream a;
    a << "I2C slave status[0x" << hex << it.first << dec << "] = " << it.second;
    fLOG(INFO, a.str()); 
  }

  clearTECErrors();
#endif
  
}


// ----------------------------------------------------------------------
driveHardware::~driveHardware() {

  fAbort = true;
  fCondition.wakeOne();

  shutDown();
}


#ifdef PI
// ----------------------------------------------------------------------
bool driveHardware::initCANSockets() {
  fLOG(INFO, "initCANSockets() start");
  // -- write CAN socket
  memset(&fFrameW, 0, sizeof(struct can_frame));
  fSw = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fSw < 0) {
    perror("socket PF_CAN failed");
    return false;
  }

  strcpy(fIfrW.ifr_name, "can0");
  int ret = ioctl(fSw, SIOCGIFINDEX, &fIfrW);
  if (ret < 0) {
    perror("ioctl failed");
    return false;
  }

  fAddrW.can_family = AF_CAN;
  fAddrW.can_ifindex = fIfrW.ifr_ifindex;
  ret = bind(fSw, (struct sockaddr *)&fAddrW, sizeof(fAddrW));
  if (ret < 0) {
    perror("bind failed");
    return false;
  }

  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  // -- read CAN socket
  memset(&fFrameR, 0, sizeof(struct can_frame));
  fSr = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fSr < 0) {
    perror("socket PF_CAN failed");
    return false;
  }

  strcpy(fIfrR.ifr_name, "can0");
  ret = ioctl(fSr, SIOCGIFINDEX, &fIfrR);
  if (ret < 0) {
    perror("ioctl failed");
    return false;
  }

  fAddrR.can_family = AF_CAN;
  fAddrR.can_ifindex = fIfrR.ifr_ifindex;
  ret = bind(fSr, (struct sockaddr *)&fAddrR, sizeof(fAddrR));
  if (ret < 0) {
    perror("bind failed");
    return false;
  }

  // -- add timeout
  struct timeval tv;
  tv.tv_sec = 0;
  // Give broadcast replies a little more time to arrive.
  tv.tv_usec = 10000; // 10 milliseconds
  if (setsockopt(fSr, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("Error in setting up time out");
  }

  fLOG(INFO, "initCANSockets() success");
  return true;
}
#endif


// ----------------------------------------------------------------------
std::string driveHardware::formatHex(unsigned int value) const {
  std::ostringstream ss;
  ss << "0x" << std::uppercase << std::hex << value;
  return ss.str();
}


// ----------------------------------------------------------------------
void driveHardware::doWarning(string errmsg, bool nothing) {

  fLOG(WARNING, errmsg);
  return;

  // -- keep code around in case we want to do something with lights
  static struct timeval tvWarningSet;

  if (nothing) {
    struct timeval tvNow;
    gettimeofday(&tvNow, 0);
    int tdiff = diff_ms(tvNow, tvWarningSet);
    if (tdiff > 10000) {
    }
  } else {
    gettimeofday(&tvWarningSet, 0);
  }

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
  fLOG(INFO, "driveHardware::doRun() entered");
  int cnt(-1);
  struct timeval tvVeryOld, tvOld, tvNew;
  gettimeofday(&tvVeryOld, 0);
  gettimeofday(&tvOld, 0);
  auto stepMs = [this](const char *label, int warnMs, int verbose, void (driveHardware::*fn)()) {
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
    (this->*fn)();
    gettimeofday(&t1, 0);
    int dt = diff_ms(t1, t0);
    if ((dt > warnMs) || (fVerbose > verbose)) {
      fLOG(INFO, string("timing: ") + label + " took " + to_string(dt) + " ms");
    }
  };

  fLOG(INFO, "driveHardware::doRun() start loop");
  while (1) {

#ifdef UZH
    evtHandler();
    if (!(readI2C()&2)){ // if odd, LED is red
      continue;
    }
#endif

    std::this_thread::sleep_for(fMilli5);
    evtHandler();
    readCAN();
    gettimeofday(&tvNew, 0);
    int tdiff = diff_ms(tvNew, tvOld);
    int tdiff2 = diff_ms(tvNew, tvVeryOld);
    if (tdiff2 > 10000.) {
      tvVeryOld = tvNew;
      // -- periodically send out everything (all = 1)
      dumpMQTT(1);
    }
    if (tdiff > 1000.) {
      tvOld = tvNew;
      if (fVerbose > 5) cout << tStamp() << " readAllParamsFromCANPublic(), tdiff = " << tdiff << endl;
      // -- read SHT85 only every 2 seconds!
      if (tdiff2 > 2000) {
        stepMs("readAirTemperature", 20, 9, &driveHardware::readAirTemperature);
        stepMs("readFlowmeter", 20, 9, &driveHardware::readFlowmeter);
        if (MAX_TEMP < 30. && fFlowMeterStatus > -1) {
          MAX_TEMP = 40.;
          SAFETY_MAXSHT85TEMP = MAX_TEMP;
          SAFETY_MAXTEMPW     = MAX_TEMP;
          SAFETY_MAXTEMPM     = MAX_TEMP;
          SHUTDOWN_TEMP       = MAX_TEMP;
          fLOG(INFO, "Flow switch activated.  Changed maximum temperatures to 40 degC");
        }
      }

      // -- read all parameters from CAN
      stepMs("readAllParamsFromCANPublic", 80, 9, &driveHardware::readAllParamsFromCANPublic);

      evtHandler();

      // -- if reconditioning had been started, go there
      if (fReconditioning > 0) doReconditioning();
      if (fHeaterStatus == 1) {
        turnOffValve(1);
        turnOffValve(0);
      }

      // -- count dount fHeaterStatus to allow cool down
      if (0 < fHeaterStatus && fHeaterStatus < HEATER_MAX_STATUS) --fHeaterStatus;

      // -- do something with the results
      if (0) cout << tStamp() << " emit signalUpdateHwDisplay tdiff = " << tdiff << endl;
      if (!fVersionOK) {
        fStatusString = "TEC f/w < 12";
      }
      emit signalUpdateHwDisplay();
      dumpCSV();
      dumpMQTT();

      // -- make sure there is no alarm before clearing
      parseCAN();

      evtHandler();

      stepMs("entertainFras", 30, 10, &driveHardware::entertainFras);
      stepMs("entertainTECs", 30, 10, &driveHardware::entertainTECs);

      stepMs("checkFan", 30, 10, &driveHardware::checkFan);
      stepMs("ensureSafety", 30, 10, &driveHardware::ensureSafety);

      if (fThrottleStatus > 0) {
        throttleN2();
      }

      ++cnt;
      // -- about once per hour?
      if (0 == cnt%3600) {
        checkDiskspace();      
      }

      // -- about once per minute
      if (0 == cnt%60) {
        stringstream a("RH/T  HYT223 T = " + to_string(fHYT223Temp) + " RH = " + to_string(fHYT223RH)
                       + (fI2CSlaveStatus[I2C_SHT85_ADDR]? ("  SHT85 T = " + to_string(fSHT85Temp) + " RH = " + to_string(fSHT85RH)): "")
                       + (fHeaterStatus > 0?"  Heater status = " + to_string(fHeaterStatus): ""));
        fLOG(INFO, a.str());

      }

      // -- print errors (if present) accumulated in CANmessage
      int nerrs = fCanMsg.nErrors();
      if (nerrs > 0) {
        fCANErrorOld = fCANErrorCounter;
        fCANErrorCounter = nerrs;
        if (redCANErrors()) {
          deque<string> errs = fCanMsg.getErrors();

          stringstream a("==CANERROR== n=" + to_string(fCANErrorCounter)
                         + " CAN frame = " + fCanMsg.getErrorFrame().getString()
                         + " errs.size() = " + std::to_string(errs.size())
                         )
            ;
          fLOG(ERROR, a.str());

          int errRepeat(0);
          int lcnt(0);
          while (errs.size() > 0) {
            string errmsg = errs.front();
            if (lcnt < 2) {
              fLOG(ERROR, "errmsg: " + errmsg); // FIXME TEMPORARY
              ++lcnt;
            }
            if (string::npos != errmsg.find("parse issue")) {
              ++errRepeat;
            }
            if (errRepeat < 2) {
              //            doWarning(errmsg);
            }
            errs.pop_front();
          }
          if (errRepeat > 2) {
            //          doWarning("truncated warning message " + to_string(errRepeat-2) + " times");
          }
          fCanMsg.clearAllFrames();
        }
      }
      //      doWarning("nothing", true);
    }
  }

}


// ----------------------------------------------------------------------
void driveHardware::ensureSafety() {
  // -- if TEC firmware is not OK, get stuck
  if (!fVersionOK) {
    fStatusString = "upgrade TEC f/w!";
    return;
  }

  if (0 == fStopOperations) {
    // fLOG(INFO, "fFlowMeterStatus = " + to_string(fFlowMeterStatus));
    if (0 == fFlowMeterStatus) {
      fStatusString = "turn on chiller!";
    } else if (fHeaterStatus > 0) {
      fStatusString = "reconditioning";
    } else {
      fStatusString = "no problem";
    }
  }

  // -- check lid status (only 1 is good enough) and set interlock accordingly
#ifdef PI
  checkLid();
  if (fOldLidStatus != fLidStatus) {
    fLOG(INFO, "changing lid status, fLidReading = " + to_string(fLidReading));
    fOldLidStatus = fLidStatus;
    if (fLidStatus < 1) {
      breakInterlock();
    } else {
      resetInterlock();
    }

    stringstream a;
    if (1 == fLidStatus) {
      a << "lid locked";
    } else if (0 == fLidStatus) {
      a << "lid unlocked";
    } else if (-1 == fLidStatus) {
      a << "lid open";
    } else {
      a << "lid status unclear";
    }
    fLOG(INFO, a.str());
    emit signalSendToServer(QString::fromStdString(a.str()));
  }
#endif

  bool greenLight(fInterlockStatus == 1?true:false);

  // -- first the trivial warnings
  if (redCANErrors() > 0) {
    stringstream a("==WARNING== CAN errors = "
                   + to_string(fCANErrorCounter) + " "
                   + fCanMsg.getErrorFrame().getString()
                   );
    fLOG(WARNING, a.str());
    if (0 == fStopOperations) fStatusString = "CANbus error";
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
  }
  if (redI2CErrors() > 0) {
    stringstream a("==WARNING== I2C errors = "
                   + to_string(fI2CErrorCounter)
                   );
    fLOG(WARNING, a.str());
    if (0 == fStopOperations) fStatusString = "I2C error";
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
  }

  // -- Check for GL condition
  if (fAirTemp < 15.0)  {
    if (0 == fStopOperations) fStatusString = "Air temp < 15";
    greenLight = false;
  }

  int allOK(0);
  // -- air temperatures
  if ((fAirTemp > SAFETY_MAXSHT85TEMP) && (0 == fHeaterStatus)) {
    greenLight = false;
    allOK = 1;
    char cs[100];
    snprintf(cs, sizeof(cs), "Air temp > %+5.2f", fAirTemp);
    if (0 == fStopOperations) fStatusString = cs;
    stringstream a("==ALARM== Box air temperature = " +
                   to_string(fAirTemp) +
                   " exceeds SAFETY_MAXSHT85TEMP = " +
                   to_string(SAFETY_MAXSHT85TEMP));
    fLOG(ERROR, a.str());
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
    emit signalAlarm(1);
    cout << "signalSetBackground(\"T\", red)" << endl;
    emit signalSetBackground("T", "red");
    stopOperations(1);
  }
  if ((fAirTemp > SHUTDOWN_TEMP) && (0 == fHeaterStatus)) {
    stopOperations(2);
  }

  // -- ensure chiller running if at least one TEC is turned on
  if (anyTECRunning()) {
    greenLight = false;
    if (fFlowMeterStatus < 0) {
      // -- should be handled by temperature checking
    } else {
      if (fFlowMeterStatus < 1) {
      
        allOK = 2;
        if (0 == fStopOperations) fStatusString = "Turn on chiller!";
        stringstream a("==ERROR== chiller not running, turn it on = ");
        fLOG(ERROR, a.str());
        emit signalSendToMonitor(QString::fromStdString(a.str()));
        emit signalSendToServer(QString::fromStdString(a.str()));
        stopOperations(3);
      }
      //FIXME?    breakInterlock();
    }
  }


  // -- dew point vs air temperature
  if ((fAirTemp - SAFETY_DPMARGIN) < fAirDP) {
    greenLight = false;
    allOK = 3;
    if (0 == fStopOperations) fStatusString = "Air temp too close to DP";
    stringstream a("==ALARM== Box air temperature = " +
                   to_string(fAirTemp) +
                   " is too close to dew point = " +
                   to_string(fAirDP)
                   );
    fLOG(ERROR, a.str());
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
    emit signalAlarm(1);
    cout << "signalSetBackground(\"DP\", red)" << endl;
    emit signalSetBackground("DP", "red");
    stopOperations(4);
  }

  // -- check water temperature (derived from TEC8; TEC8 must stay active)
  if (fTECData[8].reg["Temp_W"].value > SAFETY_MAXTEMPW) {
    greenLight = false;
    allOK = 4;
    char cs[100];
    snprintf(cs, sizeof(cs), "Water temp = %+5.2f", fTECData[8].reg["Temp_W"].value);
    if (0 == fStopOperations) fStatusString = cs;
    stringstream a("==ALARM== Water temperature = " +
                   to_string(fTECData[8].reg["Temp_W"].value) +
                   " exceeds SAFETY_MAXTEMPW = " +
                   to_string(SAFETY_MAXTEMPW));
    fLOG(ERROR, a.str());
    emit signalSendToMonitor(QString::fromStdString(a.str()));
    emit signalSendToServer(QString::fromStdString(a.str()));
    emit signalAlarm(1);
    stopOperations(5);
  }
  if (fTECData[8].reg["Temp_W"].value > SHUTDOWN_TEMP) {
    stopOperations(6);
  }      

  // -- check module temperatures (1) value and (2) against dew point
  for (int itec = 1; itec <= 8; ++itec) {
    if (0 == fActiveTEC[itec]) continue;
    double mtemp = fTECData[itec].reg["Temp_M"].value;
    if (mtemp < 15) {
      greenLight = false;
      if (0 == fStopOperations) fStatusString = "Keep lid closed";
    }
    if (mtemp > SAFETY_MAXTEMPM) {
      greenLight = false;
      allOK = 5;
      char cs[100];
      snprintf(cs, sizeof(cs), "Module %d too hot", itec);
      if (0 == fStopOperations) fStatusString = cs;
      stringstream a("==ALARM== module temperature = " +
                     to_string(mtemp) +
                     " exceeds SAFETY_MAXTEMPM = " +
                     to_string(SAFETY_MAXTEMPM));
      fLOG(ERROR, a.str());
      emit signalSendToMonitor(QString::fromStdString(a.str()));
      emit signalSendToServer(QString::fromStdString(a.str()));
      emit signalAlarm(1);
      QString qtec = QString::fromStdString("tec"+to_string(itec));
      cout << "signalSetBackground(" << qtec.toStdString() << ", red)" << endl;
      emit signalSetBackground(qtec, "red");
      stopOperations(7);
    }
    if (mtemp > SHUTDOWN_TEMP) {
      stopOperations(8);
    }      

    if ((mtemp > -90.) && (mtemp < fAirDP + SAFETY_DPMARGIN)) {
      greenLight = false;
      allOK = 6;
      stringstream a("==ALARM== module " + to_string(itec) + " temperature = " +
                     to_string(mtemp) +
                     " is too close to dew point = " +
                     to_string(fAirDP) +
                     ", air temperature = " +
                     to_string(fAirTemp)
                     );
      fLOG(ERROR, a.str());
      if (0 == fStopOperations) fStatusString = "Module " + std::to_string(itec) + " temp near DP";
      emit signalSendToMonitor(QString::fromStdString(a.str()));
      emit signalSendToServer(QString::fromStdString(a.str()));
      emit signalAlarm(1);
      QString qtec = QString::fromStdString("tec"+to_string(itec));
      cout << "signalSetBackground(" << qtec.toStdString() << ", red)" << endl;
      emit signalSetBackground(qtec, "red");
      //FIXME? breakInterlock();
    }
  }

  // -- check with water temperature whether chiller is running
  if ((-1 == fFlowMeterStatus) && (fTECData[8].reg["Temp_W"].value > 20.)) {
    if (0 == fStopOperations) fStatusString = "turn on chiller!";
  }
  
  if ((fAlarmState > 0) && (0 == allOK)) {
    cout << "allOK = " << allOK << ", alarm condition gone, reset siren and red lamp" << endl;
    cout << "set GPIORED = LOW" << endl;
#ifdef PI
    gpio_write(fPiGPIO, GPIORED, 0);
    fTrafficRed = 0;
#endif
    if (1 == fLidStatus) {
      if (fOldLidStatus != fLidStatus) {
        fLOG(INFO, "changing lid status, fLidReading = " + to_string(fLidReading));
        fOldLidStatus = fLidStatus;
        stringstream a;
        if (1 == fLidStatus) {
          a << "lid locked";
        } else if (0 == fLidStatus) {
          a << "lid unlocked";
        } else if (-1 == fLidStatus) {
          a << "lid open";
        } else {
          a << "lid status unclear";
        }
        fLOG(INFO, a.str());
        emit signalSendToServer(QString::fromStdString(a.str()));
#ifdef PI
      fOldLidStatus = fLidStatus;
      resetInterlock();
#endif
      }
    }

    emit signalKillSiren();
    emit signalSetBackground("T", "white");
    emit signalSetBackground("DP", "white");
    emit signalSetBackground("RH", "white");
    emit signalAlarm(0);
  }

#ifdef PI
  // -- During reconditioning (fHeaterStatus > 0), turn on all traffic lights
  if (fHeaterStatus > 0) {
    gpio_write(fPiGPIO, GPIOGREEN, 1);
    fTrafficGreen = 1;
    gpio_write(fPiGPIO, GPIOYELLO, 1);
    fTrafficYellow = 1;
    gpio_write(fPiGPIO, GPIORED, 1);
    fTrafficRed = 1;
  } else {
    // -- Normal traffic light control
    if (greenLight) {
      gpio_write(fPiGPIO, GPIOGREEN, 1);
      fTrafficGreen = 1;
    } else {
      gpio_write(fPiGPIO, GPIOGREEN, 0);
      fTrafficGreen = 0; 
    }

    // -- add yellow blinking light in case fan is off but conditions are not safe
    if (!getStatusFan()) {
      if (!greenLight) {
        // -- need local (static) variable because fTrafficYellow is reset in checkFan()
        static bool yelloOn(false);
        if (0 == fStopOperations) fStatusString = "Keep lid closed";
        if (yelloOn) {
          gpio_write(fPiGPIO, GPIOYELLO, 0);
          fTrafficYellow = 0; 
          yelloOn = false;        
        } else {
          gpio_write(fPiGPIO, GPIOYELLO, 1);
          fTrafficYellow = 1;
          yelloOn = true;
        }
      } else {
        gpio_write(fPiGPIO, GPIOYELLO, 0);
        fTrafficYellow = 0;
      }
    } else {
      gpio_write(fPiGPIO, GPIOYELLO, 1);
      fTrafficYellow = 1;
    }
    
    // -- Red light control (normally off unless there's an alarm)
    gpio_write(fPiGPIO, GPIORED, 0);
    fTrafficRed = 0;
  }
#endif

  // -- make sure that all TECs that should be on are on
  for (int itec = 1; itec <= 8; ++itec) {
    if (fTECTurnedOn[itec]) {
      if (0 == static_cast<int>(fTECData[itec].reg["PowerState"].value)) {
        stringstream a;
        a << "TEC " << itec << " should be on, turning it on";
        fLOG(INFO, a.str());
        turnOnTEC(itec);
      }
    }
  }

  // -- keep a record for the next time
  fAlarmState = allOK;

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
  int version(-99);
  if (0 == fActiveTEC[itec]) {
    cout << "TEC " << itec <<  " not active, skipping" << endl;
    return version;
  }
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 6; // GetSWVersion
  fCANVal = 0; // nothing required

  fMutex.lock();
  sendCANmessage(false);
  // -- robustly wait for the proper reply frame (CMD reply reg=6 for this TEC)
  bool gotVersion(false);
  for (int i = 0; i < 10; ++i) {
    readCAN(1, false);
    canFrame a = fCanMsg.getFrame();
    if ((a.fTec == static_cast<unsigned int>(itec)) && (0 == a.fType) && (6 == a.fReg)) {
      version = a.fIntVal;
      gotVersion = true;
      break;
    }
    std::this_thread::sleep_for(fMilli5);
  }
  fMutex.unlock();

  if (!gotVersion) {
    fLOG(WARNING, "getSWVersion(" + to_string(itec) + ") timed out waiting for reg 6 reply");
  }

  fSWVersionCached[itec] = version;
  stringstream sbla; sbla << "getSWVersion("
                          << itec << ")"
                          << " reg = " << fCANReg << hex
                          << " canID = 0x" << fCANId << dec
                          << " version = " << version << ".";
  fLOG(INFO, sbla.str());
  return version;
}


// ----------------------------------------------------------------------
int driveHardware::getSWVersionCached(int itec) {
  if (fSWVersionCached.find(itec) != fSWVersionCached.end()) {
    fLOG(INFO, "getSWVersionCached(" + to_string(itec) + ") = " + to_string(fSWVersionCached[itec]));
    return fSWVersionCached[itec];
  }
  return getSWVersion(itec);
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
  cout << "driveHardware::shutDown()" << endl;
  fLOG(INFO, "driveHardware::shutDown()");
  breakInterlock();

  gpio_write(fPiGPIO, GPIORED,   0);
  fTrafficRed = 0;
  gpio_write(fPiGPIO, GPIOGREEN, 0);
  fTrafficGreen = 0; 
  gpio_write(fPiGPIO, GPIOYELLO, 0);
  fTrafficYellow = 0; 

  for (int itec = 1; itec <= 8; ++itec) {
    turnOffTEC(itec);
    std::this_thread::sleep_for(fMilli5);
  }

  pigpio_stop(fPiGPIO);

  fStatusString = "Reboot!";
  return;

#endif
}


// ----------------------------------------------------------------------
void driveHardware::readCAN(int nreads, bool setMutex) {
  if (setMutex) fMutex.lock();
  int nbytes(0);
  int nGoodReads(0);

  bool DBX(false);

  fCanLastReadRequested = nreads;
  ++nreads;
  fCanLastReadAttempts = nreads;

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
      ++nGoodReads;
      if (1) {
#ifdef PI
        canFrame f(fFrameR.can_id, fFrameR.can_dlc, fFrameR.data);
        fCanMsg.addFrame(f);
#endif
      }
    }
    --nreads;
  }
  fCanLastReadReceived = nGoodReads;
  if (setMutex) fMutex.unlock();

  parseCAN();
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

  if (fVerbose > 5) cout << "answerIoGet what ->" << what << "<-" << endl;
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
  std::string regnameLower = regname;
  std::transform(regnameLower.begin(), regnameLower.end(), regnameLower.begin(),
                 [](unsigned char c){ return std::tolower(c); });
  int ntec(1);
  for (int itec = 1; itec <= 8; ++itec) {
    if ((0 != tec) && (itec != tec)) continue;
    float regValue = getTECRegister(itec, regname);
    if (0 == fActiveTEC[itec]) {
      if ((regnameLower == "powerstate") || (regnameLower == "mode") || (regnameLower == "error")) {
        regValue = 0.;
      } else {
        regValue = -99.;
      }
    }
    if (ntec > 1) str << ",";
    str << regValue;
    ++ntec;
  }
  QString qmsg = QString::fromStdString(str.str());
  emit signalSendToServer(qmsg);
  return;
}


// ----------------------------------------------------------------------
void driveHardware::answerIoSet(string &) {
  string what = fIoMessage;

  if (fVerbose > 5) fLOG(INFO, "answerIoSet what ->" + what + "<-");
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
    if (fVerbose > 9) fLOG(INFO, "register ->" + regname
         + "<- value ->" + to_string(value)
         + "<- tec = " + to_string(tec));

    for (int itec = 1; itec <= 8; ++itec) {
      if ((0 != tec) && (itec != tec)) continue;
      setTECRegister(itec, regname, value);
      if (fVerbose > 9) fLOG(INFO, "answerIoSet: setTECRegister(" + to_string(itec) + ", " + regname + ", " + to_string(value) + ")");
    }
  }
  return;
}

// ----------------------------------------------------------------------
void driveHardware::answerIoCmd() {
  string what = fIoMessage;
  if (fVerbose > 5) fLOG(INFO, "answerIoCmd what ->" + what + "<-");
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
  fLOG(INFO, "answerIoCmd cmdname = " + cmdname);

  if (string::npos != cmdname.find("RecoverCAN")) {
    fLOG(INFO, "Calling recoverCANBus() ");
    bool ok = recoverCANBus();
    QString qmsg = QString::fromStdString(string("RecoverCAN = ") + (ok ? "OK" : "FAIL"));
    emit signalSendToServer(qmsg);
    return;
  }

  if (string::npos != cmdname.find("Power_On")) {
    fCANReg = 1;
  } else if (string::npos != cmdname.find("Power_Off")) {
    fCANReg = 2;
  } else if (string::npos != cmdname.find("ClearError")) {
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
  } else if (string::npos != cmdname.find("InactivateTEC")) {
    if (1 == tec) {
      string a("==WARNING== TEC 1 cannot be inactivated (required for lid sensor safety).");
      fLOG(WARNING, a);
      emit signalSendToServer(QString::fromStdString(a));
      return;
    }
    if (8 == tec) {
      string a("==WARNING== TEC 8 cannot be inactivated (required for water temperature safety).");
      fLOG(WARNING, a);
      emit signalSendToServer(QString::fromStdString(a));
      return;
    }
    fActiveTEC[tec] = 0;
    return;
  }

  stringstream str;
  str << cmdname << " = ";
  int ntec(1);
  for (int itec = 1; itec <= 8; ++itec) {
    if ((0 != tec) && (itec != tec)) continue;
    if (0 == fActiveTEC[itec]) {
      if ((5 == fCANReg) || (6 == fCANReg) || (7 == fCANReg) || (8 == fCANReg) || (255 == fCANReg)) {
        if (ntec > 1) str << ",";
        // -- keep fixed-width all-TEC replies: mark inactive boards with unphysical sentinel
        if (6 == fCANReg) {
          str << -99;
        } else {
          str << -1;
        }
        ++ntec;
      }
      continue;
    }
    // -- use the proper functions to also control YELLO and the fan
    if (1 == fCANReg) {
      turnOnTEC(itec);
      continue;
    }
    if (2 == fCANReg) {
      turnOffTEC(itec);
      continue;
    }
    // -- the following suppresses the MQTT broadcast, I think
    // if (6 == fCANReg) {
    //   getSWVersion(itec);
    //   continue;
    // }

    fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_CMD);
    fCANVal = 0; // nothing required
    stringstream sbla; sbla << cmdname << " tec("
                            << itec << ")"
                            << " reg = " << fCANReg << hex
                            << " canID = 0x" << fCANId << dec;
    fLOG(INFO, sbla.str());
    fMutex.lock();
    sendCANmessage(false);
    int cmdResult(-99);
    bool gotCmdReply(false);
    // -- robustly wait for the proper command reply
    for (int iwait = 0; iwait < 10; ++iwait) {
      readCAN(1, false);
      canFrame a = fCanMsg.getFrame();
      if ((a.fTec == static_cast<unsigned int>(itec)) && (0 == a.fType) && (static_cast<unsigned int>(fCANReg) == a.fReg)) {
        cmdResult = a.fIntVal;
        gotCmdReply = true;
        break;
      }
      std::this_thread::sleep_for(fMilli5);
    }
    fMutex.unlock();

    if ((6 == fCANReg) && !gotCmdReply) {
      fLOG(WARNING, "answerIoCmd(GetSWVersion) timed out waiting for reply from tec " + to_string(itec));
    }
    if (5 == fCANReg) {
      if (ntec > 1) str << ",";
      str << itec;
      ++ntec;
    } else if (6 == fCANReg) {
      if (ntec > 1) str << ",";
      str << cmdResult;
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

  // -- Ignore all messages except for heatOff when reconditioning
  if (fHeaterStatus > 0) {
    if (string::npos != fIoMessage.find("> ")) {

    } else if (string::npos != fIoMessage.find("cmd ")) {
      s1 = "heatOff";  s2 = "heatoff"; s3 = "cmd";
      if (findInIoMessage(s1, s2, s3)) {
        heatHYT223(0);      
        return;
      }
    }

    stringstream sbla; 
    sbla << "Reconditioning in progress, heater status: " << fHeaterStatus;
    sbla << " ignoring ->" << fIoMessage << "<-";
    fLOG(INFO, sbla.str());
    return;
  }

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

    s1 = "throttleN2"; s2 = "throttle";
    if (findInIoMessage(s1, s2, s3)) {
      stringstream str;
      str << "throttle" << " = " << (getThrottleStatus()?"on":"off");
      QString qmsg = QString::fromStdString(str.str());
      emit signalSendToServer(qmsg);
    }

    for (int i = 1; i < 9; ++i) {
      stringstream str1;
      str1 << "vprobe" << i;
      s1 = str1.str(); s2 = str1.str();
      if (findInIoMessage(s1, s2, s3)) {
        stringstream str;
        readVProbe(i);
        str << fVprobeVoltages;
        QString qmsg = QString::fromStdString(str.str());
        emit signalSendToServer(qmsg);
      }
    }

    // -- get the ground voltages of the last vprobe readout
    s1 = "vprobegnd"; s2 = "vprobegnd";
    if (findInIoMessage(s1, s2, s3)) {
      stringstream str;
      readVProbeGnd();
      str << fVprobeGndVoltages;
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

    s1 = "monitoring"; s2 = "allMonTessie";
    if (findInIoMessage(s1, s2, s3)) {
      fLOG(INFO, "calling dumpMQTT(1) from ctrlTessie command");
      dumpMQTT(1);
    }


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
    s1 = "InactivateTEC";  s2 = "inactivatetec";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "Power_On";  s2 = "Power_On";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "Power_Off";  s2 = "Power_Off";
    if (findInIoMessage(s1, s2, s3)) {
      answerIoCmd();
    }

    s1 = "valve0";  s2 = "valve0";
    if (findInIoMessage(s1, s2, s3)) {
      toggleFras(1);
    }

    s1 = "heatOn";  s2 = "heaton";
    if (findInIoMessage(s1, s2, s3)) {
      heatHYT223(1);      
    }

    s1 = "heatOff";  s2 = "heatoff";
    if (findInIoMessage(s1, s2, s3)) {
      heatHYT223(0);      
    }

    s1 = "startReconditioning";  s2 = "startReconditioning";
    if (findInIoMessage(s1, s2, s3)) {
      doReconditioning();      
    }

    s1 = "throttleN2On";  s2 = "throttleOn";
    if (findInIoMessage(s1, s2, s3)) {
      fThrottleStatus = 1; 
    }

    s1 = "throttleN2Off";  s2 = "throttleOff";
    if (findInIoMessage(s1, s2, s3)) {
      fThrottleStatus = 0; 
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

    s1 = "RecoverCAN"; s2 = "recovercan";
    if (findInIoMessage(s1, s2, s3)) {
      recoverCANBus();
    }

    s1 = "PowerCycle3V3"; s2 = "powercycle3v3";
    if (findInIoMessage(s1, s2, s3)) {
      powerCycle3V3();
    }

    s1 = "PowerOff3V3"; s2 = "poweroff3v3";
    if (findInIoMessage(s1, s2, s3)) {
      power3V3(false);
    }

    s1 = "PowerOn3V3"; s2 = "poweron3v3";
    if (findInIoMessage(s1, s2, s3)) {
      power3V3(true);
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
    vhelp.push_back("> hostname: " + fHostName);
    vhelp.push_back("> thread:   ctrlTessie");
    vhelp.push_back("> ===================");
    vhelp.push_back("> ");
    vhelp.push_back("> Note: [tec {0|x}] can be before or after {get|set|cmd XXX}, e.g.");
    vhelp.push_back(">       cmd Power_On tec 7");
    vhelp.push_back(">       tec 7 cmd Power_Off");
    vhelp.push_back("> ");
    vhelp.push_back("> Note: tec numbering is from 1 .. 8. tec 0 refers to all TECs.");
    vhelp.push_back("> ");
    vhelp.push_back("> Note: Once heatOn has been called, tessie will only react to heatOff");
    vhelp.push_back("> ");
    vhelp.push_back("> cmd messages:");
    vhelp.push_back("> -------------");
    vhelp.push_back("> cmd valve0");
    vhelp.push_back("> cmd valve1");
    vhelp.push_back("> cmd throttleN2On");
    vhelp.push_back("> cmd throttleN2Off");
    vhelp.push_back("> cmd heatOn");
    vhelp.push_back("> cmd heatOff");
    vhelp.push_back("> cmd startReconditioning");
    vhelp.push_back("> cmd RecoverCAN");
    vhelp.push_back("> cmd PowerCycle3V3");
    vhelp.push_back("> cmd PowerOff3V3");
    vhelp.push_back("> cmd PowerOn3V3");
    vhelp.push_back("> [tec {0|x}] cmd Power_On");
    vhelp.push_back("> [tec {0|x}] cmd Power_Off");
    vhelp.push_back("> [tec {0|x}] cmd ClearError");
    vhelp.push_back("> [tec {0|x}] cmd GetSWVersion");
    vhelp.push_back("> [tec {0|x}] cmd SaveVariables");
    vhelp.push_back("> [tec {0|x}] cmd LoadVariables");
    vhelp.push_back("> [tec {0|x}] cmd Reboot");
    vhelp.push_back("> [tec {0|x}] cmd InactivateTEC");

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
    vhelp.push_back("> get vprobe[1-8]");
    vhelp.push_back("> get vprobegnd");
    vhelp.push_back("> ");
    vhelp.push_back("> get {monitoring|allMonTessie}");
    vhelp.push_back("> ");


    vhelp.push_back("> [tec {0|x}] get Mode");
    vhelp.push_back("> [tec {0|x}] get ControlVoltage_Set");
    vhelp.push_back("> [tec {0|x}] get PID_kp");
    vhelp.push_back("> [tec {0|x}] get PID_ki");
    vhelp.push_back("> [tec {0|x}] get PID_kd");
    vhelp.push_back("> [tec {0|x}] get Temp_Set");
    vhelp.push_back("> [tec {0|x}] get PID_Max");
    vhelp.push_back("> [tec {0|x}] get PID_Min");

    vhelp.push_back("> [tec {0|x}] get Temp_W");
    vhelp.push_back("> [tec {0|x}] get Temp_M");
    vhelp.push_back("> [tec {0|x}] get Temp_Diff");

    vhelp.push_back("> [tec {0|x}] get Peltier_U");
    vhelp.push_back("> [tec {0|x}] get Peltier_I");
    vhelp.push_back("> [tec {0|x}] get Peltier_R");
    vhelp.push_back("> [tec {0|x}] get Peltier_P");

    vhelp.push_back("> [tec {0|x}] get Supply_U");
    vhelp.push_back("> [tec {0|x}] get Supply_I");
    vhelp.push_back("> [tec {0|x}] get Supply_P");

    vhelp.push_back("> [tec {0|x}] get PowerState");
    vhelp.push_back("> [tec {0|x}] get Error");
    vhelp.push_back("> [tec {0|x}] get Ref_U");

    vhelp.push_back("> Tutorial for getting started:");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \" set valve0 on\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"set valve1 on\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"set ControlVoltage_Set 4.5\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"cmd Power_On\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"cmd Power_Off\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"set ControlVoltage_Set 0.0\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"set valve0 off\" ");
    vhelp.push_back("> mosquitto_pub -h " + fHostName + " -t \"ctrlTessie\" -m \"set valve1 off\" ");

    for (unsigned int i = 0; i < vhelp.size(); ++i) {
      emit signalSendToServer(QString::fromStdString(vhelp[i]));
    }
  }
  fIoMessage = "";
}


// ----------------------------------------------------------------------
void driveHardware::sendCANmessage(bool setMutex) {

#ifdef PI
  int itec = 0;
  itec = fCANId & 0xf;
  if (fVerbose > 9) cout << "sendCANmessage() TEC " << itec << endl;

  if ((itec > 0) && (0 == fActiveTEC[itec])) {
    if (fVerbose > 9) cout << "TEC " << itec <<  " not active, skipping" << endl;
    return;
  }

  char data[4] = {0, 0, 0, 0};
  fFrameW.can_id = fCANId;
  fCanLastSentId = fCANId;
  fCanLastSentReg = fCANReg;
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
      if (fVerbose > 9) cout << "DBX interpreting as unsigned int ->" << intCanVal << "<-" << endl;
      memcpy(data, &intCanVal, sizeof intCanVal);
    } else {
      memcpy(data, &fCANVal, sizeof fCANVal);
    }
    fFrameW.data[1] = data[0];
    fFrameW.data[2] = data[1];
    fFrameW.data[3] = data[2];
    fFrameW.data[4] = data[3];
  }

  if (fVerbose > 9) {
    if (1 == command) {
      fLOG(INFO, "   sendCANmessage: canid = " + to_string(fCANId) + " cmd = " + to_string(fCANReg));
    } else {
      fLOG(INFO, "   canid = " + to_string(fCANId) + " reg = " + to_string(fCANReg)
           + " value = " + to_string(fCANVal)
           + " dlength = " + to_string(dlength));
    }
    fLOG(INFO, "    can_id  = " + formatHex(fFrameW.can_id) + " (from sendCANmessage())");
    fLOG(INFO, "    can_dlc = " + to_string(fFrameW.can_dlc));
    for (int i = 0; i < fFrameW.can_dlc; ++i) {
      fLOG(INFO, "    data[" + to_string(i) + "] = " + to_string(fFrameW.data[i])
           + " / " + formatHex(static_cast<unsigned int>(fFrameW.data[i])));
    }
  }

  if (setMutex) fMutex.lock();

  //6.Send message
  setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
  if (nbytes != sizeof(fFrameW)) {
    printf("CAN BUS Send Error frame[0]!\r\n");
  }

  // -- this is required to absorb the write request from fSr
  nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
  fCanLastAbsorbRead = (nbytes > -1 ? 1 : 0);

  // -- wait a bit
  std::this_thread::sleep_for(fMilli5);

  if (setMutex) fMutex.unlock();

#endif
}


// ----------------------------------------------------------------------
bool driveHardware::recoverCANBus() {
  fLOG(WARNING, "calling CANmessage::clearAll() in recoverCANBus()");
  fCanMsg.clearAll();
  #ifdef PI
  fLOG(WARNING, "driveHardware::recoverCANBus() start");

  auto runCmd = [this](const std::string &cmd) -> bool {
    int rc = std::system(cmd.c_str());
    if (0 == rc) return true;

    // -- fallback for deployments where ctrlTessie runs without CAP_NET_ADMIN
    std::string sudoCmd = "sudo -n " + cmd;
    rc = std::system(sudoCmd.c_str());
    if (0 != rc) {
      fLOG(WARNING, "CAN recovery command failed: " + cmd
           + " rc=" + to_string(rc)
           + " (also failed with sudo -n)");
      return false;
    }
    return true;
  };

  fMutex.lock();

  if (fSr >= 0) {
    close(fSr);
    fSr = -1;
  }
  if (fSw >= 0) {
    close(fSw);
    fSw = -1;
  }

  bool ok = true;
  ok &= runCmd("ip link set can0 down");
  // Try controller-level restart and set auto-restart.
  ok &= runCmd("ip link set can0 type can restart-ms 100");
  ok &= runCmd("ip link set can0 up");

  if (ok) {
    ok = initCANSockets();
  }

  fMutex.unlock();

  fLOG(ok ? INFO : ERROR, string("driveHardware::recoverCANBus() ") + (ok ? "success" : "failed"));
  fLOG(WARNING, "calling CANmessage::clearAll() again in recoverCANBus()");
  fCanMsg.clearAll();
  return ok;
#else
  fLOG(WARNING, "driveHardware::recoverCANBus() only available on PI build");
  return false;
#endif
}


// ----------------------------------------------------------------------
void driveHardware::entertainTECs() {
#ifdef PI
  fCANId = (CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 3; // Watchdog
  fCANVal = fTECParameter;
  sendCANmessage();
#endif
}


// ----------------------------------------------------------------------
void driveHardware::entertainFras() {
#ifdef PI
  if (fMutex.tryLock()) {
  } else {
    return;
  }
  //  fMutex.lock();
  if (fVerbose > 0) cout << "entertainFras() fRelaisMask = " << fRelaisMask << endl;
  if (0 == fRelaisMask) {
    if (fVerbose > 9) cout << "entertainFras() fRelaisMask = 0, sending RTR frame" << endl;
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
    fFrameW.data[0] = fRelaisMask;

    // -- Send message
    setsockopt(fSw, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    int nbytes = write(fSw, &fFrameW, sizeof(fFrameW));
    if (nbytes != sizeof(fFrameW)) {
      if (fVerbose > 0) printf("CAN BUS Send Error frame[0]!\r\n");
    }
  }
  // -- this is required to absorb the write request from fSr
  int nbytes = read(fSr, &fFrameR, sizeof(fFrameR));
  fMutex.unlock();
#endif
}


// ----------------------------------------------------------------------
void driveHardware::turnOnFan() {
#ifndef UZH
  if (!getStatusFan()) toggleFras(4);
#endif
}


// ----------------------------------------------------------------------
void driveHardware::turnOffFan() {
#ifndef UZH
  if (getStatusFan()) toggleFras(4);
#endif
}


// ----------------------------------------------------------------------
void driveHardware::turnOnLV() {
  if (!getStatusLVInterlock()) toggleFras(8);
  //fLOG(WARNING, "turning on LV");
}


// ----------------------------------------------------------------------
void driveHardware::turnOffLV() {
  if (getStatusLVInterlock()) toggleFras(8);
  //fLOG(WARNING, "turning off LV");
}


// ----------------------------------------------------------------------
void driveHardware::power3V3(bool on) {
  stringstream a("power3V3(" + to_string(on) + ")");
  if (fVerbose > -1)fLOG(INFO, a.str()); 
  if (on) {
    gpio_write(fPiGPIO, GPIOPSUEN, 1);
  } else {
    gpio_write(fPiGPIO, GPIOPSUEN, 0);
  }
}


// ----------------------------------------------------------------------
void driveHardware::powerCycle3V3(int n100ms) {
  power3V3(false);
  std::this_thread::sleep_for(fMilli100*n100ms);
  power3V3(true);
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
  fMutex.lock();
  int old = fRelaisMask;
  fRelaisMask = old xor imask;

  if (fVerbose > 1) fLOG(INFO, "toggleFras(" + to_string(imask) + ") entered");
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
  fFrameW.data[0] = fRelaisMask;
#endif

  string sstatus("");
  if (1 == imask) {
    if (getStatusValve0()) {
      sstatus = "on";
    } else {
      sstatus = "off";
    }
  }
  if (2 == imask) {
    if (getStatusValve1()) {
      sstatus = "on";
    } else {
      sstatus = "off";
    }
  }
  if (4 == imask) {
    if (getStatusFan()) {
      sstatus = "on";
    } else {
      sstatus = "off";
    }
  }
  stringstream sbla;
  if ((1 == imask) || (2 == imask)) {
    sbla << "toggleFRAS(" << (imask == 1? "flush) ": "rinse) ") << sstatus;
  } else {
#ifndef UZH
    sbla << "toggleFRAS(fan) " << sstatus;
#endif
  }
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
  fMutex.unlock();

#ifdef UZH
  if (4 == imask){
    if (fBusyDisarm){
      fBusyDisarm = false;
    } else{
      fBusyDisarm = true;
      std::this_thread::sleep_for(400ms);
      toggleFras(4);
    }
  }
#endif

}


// ----------------------------------------------------------------------
void driveHardware::stopOperations(int icode) {
  if (0 == fStopOperations) { 
    fStopOperations = icode;
    fLOG(INFO, "stopOperations(" + to_string(icode) +  ") invoked");
    fStatusString = "emergency";

#ifdef PI
    breakInterlock();
#endif
    
    for (int itec = 1; itec <= 8; ++itec) {
      if (0 == fActiveTEC[itec]) continue;
      if (1 == static_cast<int>(fTECData[itec].reg["PowerState"].value)) {
        turnOffTEC(itec);
      }
      std::this_thread::sleep_for(fMilli5);
    }

    turnOnValve(0);
    turnOnValve(1);
  }

  stringstream a("==ALARM== Emergency stop T(air) = " +
                 to_string(fAirTemp) +
                 " T(water) = " + to_string(fTECData[8].reg["Temp_W"].value));
  fLOG(ERROR, a.str());
  emit signalSendToMonitor(QString::fromStdString(a.str()));
  emit signalSendToServer(QString::fromStdString(a.str()));
  emit signalAlarm(1);

}


// ----------------------------------------------------------------------
float driveHardware::getTECRegister(int itec, std::string regname) {
  if (0 == fActiveTEC[itec]) {
    return -999.;
  }
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
bool driveHardware::isTECActive(int itec) {
  auto it = fActiveTEC.find(itec);
  if (it == fActiveTEC.end()) return false;
  return (1 == it->second);
}


// ----------------------------------------------------------------------
void  driveHardware::turnOnTEC(int itec) {
  if (fVerbose > 1) fLOG(INFO, "turnOnTEC(" + to_string(itec) + ") entered");
  if (0 == fFlowMeterStatus) {
    string a("==HINT== chiller not running, turn on chiller flow!"); 
    fLOG(INFO, a);
    emit signalSendToServer(QString::fromStdString(a));
    emit signalSendToMonitor(QString::fromStdString(a));
    return;
  }
    
  float mtemp = fTECData[8].reg["Temp_W"].value;
  if ((fFlowMeterStatus < 0) && (mtemp > 20.0)) {
    char cs[200];
    snprintf(cs, sizeof(cs), "==HINT== water temperature = %+5.2f indicates chiller off. Turn it on!", mtemp);
    string a(cs); 
    fLOG(INFO, a);
    emit signalSendToServer(QString::fromStdString(a));
    emit signalSendToMonitor(QString::fromStdString(a));
    return;
  }

  if (0 == fActiveTEC[itec]) {
    if (fVerbose > 1) fLOG(INFO, "TEC " + to_string(itec) +  " not active, skipping");
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

  fTECData[itec].reg["PowerState"].value = 1.;
  fTECTurnedOn[itec] = true;
#ifdef PI
  gpio_write(fPiGPIO, GPIOYELLO, 1);
  fTrafficYellow = 1;
#endif

  if (!getStatusFan()) {
    turnOnFan();
  }
}


// ----------------------------------------------------------------------
void  driveHardware::turnOffTEC(int itec) {
  if (0 == fActiveTEC[itec]) {
    if (fVerbose > 1) fLOG(INFO, "TEC " + to_string(itec) +  " not active, skipping");
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

  fTECTurnedOn[itec] = false;
  checkFan();
}


// ----------------------------------------------------------------------
bool driveHardware::anyTECRunning() {
  bool oneRunning(false);
  for (int itec = 1; itec <= 8; ++itec) {
    if (0 == fActiveTEC[itec]) continue;
    if (1 == static_cast<int>(fTECData[itec].reg["PowerState"].value)) {
      oneRunning = true;
      break;
    }
  }
  return oneRunning;
}


// ----------------------------------------------------------------------
// -- check whether any TEC is still turned on. If not, turn off fan.
void driveHardware::checkFan() {
  bool oneRunning = anyTECRunning();
  if (oneRunning) {
    // do nothing
  } else {
#ifdef PI
    gpio_write(fPiGPIO, GPIOYELLO, 0);
    fTrafficYellow = 0; 
#endif
    turnOffFan();
  }
}


// ----------------------------------------------------------------------
float driveHardware::getTECRegisterFromCAN(int itec, std::string regname) {
  if (fVerbose > 5) fLOG(INFO, "getTECRegisterFromCAN regname ->" + regname + "<-");
  if (itec > 0) {
    if (0 == fActiveTEC[itec]) {
      if (regname == "PowerState") {
        return 0.;
      }
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
  fMutex.lock();
  sendCANmessage(false);
  if (itec > 0) {
    std::this_thread::sleep_for(fMilli5);
  } else {
    std::this_thread::sleep_for(fMilli5);
  }
  if (0) fLOG(INFO, "  getTECRegisterFromCAN for tec = " + to_string(itec)
              + " register = " + regname
              + " regidx = " + to_string(fCANReg)
              );

  if (itec > 0) {
    readCAN(1, false);
    fMutex.unlock();
    fCANReadFloatVal = fCanMsg.getFloat(itec, fCANReg);
    return fCANReadFloatVal;
  } else {
    readCAN(fNActiveTEC, false);
    fMutex.unlock();
    if (fCanLastReadReceived < fNActiveTEC) {
      ++fCanShortfallCount;
      // -- keep this compact: print details every Nth shortfall
      if ((fCanShortfallCount <= 3) || (0 == fCanShortfallCount%25)) {
        stringstream sbla;
        sbla << "CAN frame shortfall #" << fCanShortfallCount
             << " in broadcast read: reg="
             << regname
             << " regIdx=" << fCANReg
             << " requested=" << fCanLastReadRequested
             << " attempts=" << fCanLastReadAttempts
             << " received=" << fCanLastReadReceived
             << " absorbRead=" << fCanLastAbsorbRead
             << " sentId=0x" << hex << fCanLastSentId << dec
             << " sentReg=" << fCanLastSentReg;
        fLOG(WARNING, sbla.str());
      }
    } else {
      // -- emit occasional recovery marker after a burst
      if (fCanShortfallCount > 0) {
        stringstream sbla;
        sbla << "CAN frame shortfall recovered after " << fCanShortfallCount << " shortfalls";
        fLOG(INFO, sbla.str());
        fCanShortfallCount = 0;
      }
    }
    return -97;
  }
  return -96;
}


// ----------------------------------------------------------------------
void driveHardware::setTECRegister(int itec, std::string regname, float value) {
  if (0 == fActiveTEC[itec]) {
    if (fVerbose > -1) fLOG(INFO, "TEC " + to_string(itec) +  " not active, skipping");
    return;
  }
  fTECData[itec].reg[regname].value = value;

  // -- program parameter
  fCANId = (itec | CANBUS_SHIFT | CANBUS_PRIVATE | CANBUS_TECREC | CANBUS_WRITE);

  fCANReg = fTECData[itec].getIdx(regname);

  fCANVal = value;

  QString aline = QString("write TEC ") +  QString::number(itec, 'd', 0)
    + QString(" register ") + QString::number(fCANReg, 'd', 0)
    + QString(" value =  ") + QString::number(fCANVal, 'f', 2)
    + QString(", canID = 0x") + QString::number(fCANId, 'x', 0)
    ;

  fLOG(INFO, aline.toStdString().c_str());

  sendCANmessage();
}


// ----------------------------------------------------------------------
void driveHardware::loadFromFlash() {
  fCANId = (CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 8; // Load Variables
  fCANVal = fTECParameter;

  QString aline = QString("load settings from FLASH");
  fLOG(INFO, aline.toStdString().c_str());

  sendCANmessage();
}


// ----------------------------------------------------------------------
void driveHardware::saveToFlash() {
  fCANId = (CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 7; // Save Variables
  fCANVal = fTECParameter;
  sendCANmessage();
}


// ----------------------------------------------------------------------
void driveHardware::clearTECErrors() {
  fCANId = (CANBUS_SHIFT | CANBUS_PUBLIC | CANBUS_TECREC | CANBUS_CMD);
  fCANReg = 5; // clear TEC error
  fCANVal = fTECParameter;
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
  b = {0,      "Mode",                 0, 1}; tdata.reg.insert(make_pair(b.name, b));
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

  // -- another read/write register
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
  auto readBroadcastFloat = [this](const std::string &regname) {
    if (fVerbose > 5) fLOG(INFO, "reading broadcast " + regname);
    getTECRegisterFromCAN(0, regname);
    int regIdx = fTECData[1].getIdx(regname);
    const bool dbgBroadcast = (fVerbose > 5);
    std::stringstream perTec;
    if (dbgBroadcast) perTec << "broadcast results " << regname << ": ";
    for (int i = 1; i <= 8; ++i) {
      if (0 == fActiveTEC[i]) {
        fTECData[i].reg[regname].value = -99.;
        if (dbgBroadcast) perTec << "TEC" << i << "=INACTIVE ";
        continue;
      }
      bool haveFrame = (fCanMsg.nFrames(i, regIdx) > 0);
      float regValue = fCanMsg.getFloat(i, regIdx);
      fTECData[i].reg[regname].value = regValue;
      if (dbgBroadcast) {
        perTec << "TEC" << i << "=" << (haveFrame ? to_string(regValue) : "MISS") << " ";
      }
    }
    if (dbgBroadcast) fLOG(INFO, perTec.str());
  };

  auto readBroadcastInt = [this](const std::string &regname) {
    if (fVerbose > 5) fLOG(INFO, "reading broadcast " + regname);
    getTECRegisterFromCAN(0, regname);
    int regIdx = fTECData[1].getIdx(regname);
    std::vector<int> missingTec;
    for (int i = 1; i <= 8; ++i) {
      if (0 == fActiveTEC[i]) continue;
      if (0 == fCanMsg.nFrames(i, regIdx)) missingTec.push_back(i);
    }
    if (!missingTec.empty()) {
      std::stringstream ss;
      ss << "Missing CAN reply for register " << regname << " (idx=" << regIdx << ") from TEC";
      if (missingTec.size() > 1) ss << "s";
      ss << ": ";
      for (size_t j = 0; j < missingTec.size(); ++j) {
        if (j > 0) ss << ",";
        ss << missingTec[j];
      }
      fLOG(WARNING, ss.str());
    }
    const bool dbgBroadcast = (fVerbose > 5);
    std::stringstream perTec;
    if (dbgBroadcast) perTec << "broadcast results " << regname << ": ";
    for (int i = 1; i <= 8; ++i) {
      if (0 == fActiveTEC[i]) {
        fTECData[i].reg[regname].value = -1;
        if (dbgBroadcast) perTec << "TEC" << i << "=INACTIVE ";
        continue;
      }
      bool haveFrame = (fCanMsg.nFrames(i, regIdx) > 0);
      int regValue = fCanMsg.getInt(i, regIdx);
      fTECData[i].reg[regname].value = regValue;
      if (dbgBroadcast) {
        perTec << "TEC" << i << "=" << (haveFrame ? to_string(regValue) : "MISS") << " ";
      }
    }
    if (dbgBroadcast) fLOG(INFO, perTec.str());
  };

  // -- fast-changing registers: always read each cycle
  readBroadcastFloat("ControlVoltage_Set");
  readBroadcastFloat("Temp_Set");
  // -- water temperature from special TEC 8 and pressure sensor from special TEC 1
  fTECData[8].reg["Temp_W"].value = getTECRegisterFromCAN(8, "Temp_W");
  if (fVerbose > 5) fLOG(INFO, "read single register Temp_W for water temperature = " + to_string(fTECData[8].reg["Temp_W"].value));
  fTECData[1].reg["Temp_W"].value = getTECRegisterFromCAN(1, "Temp_W");
  if (fVerbose > 5) fLOG(INFO, "read single register Temp_W for pressure sensor = " + to_string(fTECData[1].reg["Temp_W"].value));
  readBroadcastFloat("Temp_M");
  readBroadcastFloat("Peltier_U");
  readBroadcastFloat("Peltier_I");
  readBroadcastFloat("Peltier_R");
  readBroadcastFloat("Peltier_P");
  readBroadcastFloat("Supply_U");
  readBroadcastFloat("Supply_I");
  readBroadcastFloat("Supply_P");

  // -- slow-changing registers: read one per cycle in round-robin (~10s cadence)
  static const std::vector<std::string> slowRegs = {
    "PID_kp", "PID_ki", "PID_kd", "PID_Max", "PID_Min",
    "Temp_Diff", "Ref_U", "Mode", "PowerState", "Error"
  };
  const std::string &slowReg = slowRegs[static_cast<size_t>(fRunCnt) % slowRegs.size()];
  if ("Temp_Diff" == slowReg) {
    fTECData[8].reg["Temp_Diff"].value = getTECRegisterFromCAN(8, "Temp_Diff");
    if (fVerbose > 5) fLOG(INFO, "read single register Temp_Diff = " + to_string(fTECData[8].reg["Temp_Diff"].value));
  } else if (("Mode" == slowReg) || ("PowerState" == slowReg) || ("Error" == slowReg)) {
    readBroadcastInt(slowReg);
  } else {
    readBroadcastFloat(slowReg);
  }
}


// ----------------------------------------------------------------------
void driveHardware::dumpMQTT(int all) {
  static map<int, TECData> oldTECData;

  // -- dump singular environmental data
  stringstream ss;
  ss << "Env = "
     << fAirTemp << ", "
     << fTECData[8].reg["Temp_W"].value << ", "
     << fAirRH << ", "
     << fAirDP << ", "
     << fCANErrorCounter << ", "
     << fI2CErrorCounter << ", "
     << getRunTime() << ", "
     << getStatusValve0() << ", "
     << getStatusValve1() 
    ;
  if (all > -1) emit signalSendToMonitor(QString::fromStdString(ss.str()));

  // -- dump various status information (disk space, traffic light, ...)
  ss.str(std::string());
  ss << "VAR = G"
     << fTrafficGreen << ", Y"
     << fTrafficYellow << ", R"
     << fTrafficRed << ", L"
     << fLidStatus << ", I"
     << fInterlockStatus << ", D"
     << fFreeDiskspace << ", F"
     << fFlowMeterStatus << ", T"
     << fThrottleStatus << ", H"
     << fHeaterStatus
    ;
  emit signalSendToMonitor(QString::fromStdString(ss.str()));

  
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

  fMonString = "";
  int cnt(0);
  for (auto const &skey: tolerances) {
    stringstream ss;
    ss << skey.first << " = ";
    bool printit(false);
    bool isInt(false); 
    bool isHex(false); 
    double inactiveValue(-99.);
    if (skey.first == "Mode")  isInt = true;
    if (skey.first == "PowerState")  isInt = true;
    if (skey.first == "Error") isHex = true;
    if (isInt || isHex) inactiveValue = -1.;

    for (int i = 1; i <= 8; ++i) {
      double value = fTECData[i].reg[skey.first].value;
      double oldValue = oldTECData[i].reg[skey.first].value;
      if (0 == fActiveTEC[i]) {
        value = inactiveValue;
        oldValue = inactiveValue;
      }
      if (isInt) {
        ss << static_cast<int>(value);
      } else if (isHex) {
        ss << "0x" << hex << static_cast<int>(value) << dec;
      } else {
        ss << value;
      }
      if (1 == all) {
        printit = true;
      } else {
        if (fabs(value - oldValue) > tolerances[skey.first]) {
          printit = true;
        }
      }
      if (i < 8) ss << ",";
    }
    if (cnt++ > 0) fMonString += " ";
    fMonString += ss.str();
    if (printit)  emit signalSendToMonitor(QString::fromStdString(ss.str()));
  }

  oldTECData = fTECData;

}


// ----------------------------------------------------------------------
void driveHardware::dumpCSV() {
  stringstream output;
  char cs[100];
  snprintf(cs, sizeof(cs), "%+5.2f,%05.2f,%+5.2f", fAirTemp, fAirRH, fAirDP);
  output << timeStamp() << "," << cs;

  // -- only one water temperature reading
  for (int i = 8; i <= 8; ++i) {
    double val = fActiveTEC[i] ? fTECData[i].reg["Temp_W"].value : -99.0;
    snprintf(cs, sizeof(cs), "%+4.1f", val);
    output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    double val = fActiveTEC[i] ? fTECData[i].reg["PowerState"].value : -1.0;
    snprintf(cs, sizeof(cs), "%1.0f", val);
    output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    double val = fActiveTEC[i] ? fTECData[i].reg["ControlVoltage_Set"].value : -99.0;
    snprintf(cs, sizeof(cs), "%+5.2f", val);
    output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    double val = fActiveTEC[i] ? fTECData[i].reg["Temp_Set"].value : -99.0;
    snprintf(cs, sizeof(cs), "%+4.1f", val);
    output << "," << cs;
  }

  for (int i = 1; i <= 8; ++i) {
    double val = fActiveTEC[i] ? fTECData[i].reg["Temp_M"].value : -99.0;
    snprintf(cs, sizeof(cs), "%+5.2f", val);
    output << "," << cs;
  }

  fCsvFile <<  output.str() << endl;
}


// ----------------------------------------------------------------------
string driveHardware::timeStamp(bool filestamp) {
  return fLOG.timeStamp(filestamp);
}


// ----------------------------------------------------------------------
void driveHardware::readAirTemperature() {
  if (fI2CSlaveStatus[I2C_SHT85_ADDR]) {
    readSHT85();
    fAirTemp = fSHT85Temp;
    fAirRH   = fSHT85RH;
    fAirDP   = fSHT85DP;
    if (fVerbose > 5) {
      fLOG(INFO, "readSHT85: T: " + to_string(fSHT85Temp) + " RH: " + to_string(fSHT85RH) + " DP: " + to_string(fSHT85DP));
    }
  }
  if (fI2CSlaveStatus[I2C_HYT223_ADDR]) {
    readHYT223();
    fAirTemp = fHYT223Temp;
    fAirRH   = fHYT223RH;
    fAirDP   = fHYT223DP;
    if (fVerbose > 5) {
      fLOG(INFO, "readHYT223: T: " + to_string(fHYT223Temp) + " RH: " + to_string(fHYT223RH) + " DP: " + to_string(fHYT223DP));
    }
  }
}


// ----------------------------------------------------------------------
void driveHardware::readHYT223() {
#ifdef PI
  int handle = i2c_open(fPiGPIO, I2CBUS, I2C_HYT223_ADDR, 0);

  for (int i = 0; i < 4; ++i) {
    fHYT223Data[i] = 0;
  }
  int length = i2c_read_device(fPiGPIO, handle, fHYT223Data, 4);
  if (length == PI_I2C_READ_FAILED) {
    i2c_close(fPiGPIO, handle);
    recoverI2CBus();
    handle = i2c_open(fPiGPIO, I2CBUS, I2C_HYT223_ADDR, 0);
    length = i2c_read_device(fPiGPIO, handle, fHYT223Data, 4);
  }
  if (length < 6) {
    unsigned int vrh  = ((fHYT223Data[0]<<8) + fHYT223Data[1]) & 0x3fff;
    unsigned int vtt  = ((fHYT223Data[2]<<8) + fHYT223Data[3]) >>2;
    
    // -- see p.13 of "AHHeatedHYT223_E2.3.1 | App Note | Humidity Modules HYT"
    fHYT223RH   = 0.00610389 * vrh;       // RH [%] = (100 / (2^{14} - 1)) * RHraw
    fHYT223Temp = 0.0100714 * vtt - 40.;  // T [degC] = (165 / (2^{14} - 1)) * Traw - 40

    // -- apply offset deduction 
    fHYT223RH -= 2.02;
    if (fHYT223RH < 0.) fHYT223RH = 0.001;  

    // -- DBX
    if (fHYT223Temp > 124.) {
      stringstream a("## HYT223 UNPHYSICAL Temperature length = " + to_string(length) + " resetting to 25.");
      fLOG(WARNING, a.str());
      fHYT223Temp = 25.;
    }

    fHYT223DP   = calcDP(fHYT223Temp, fHYT223RH, 1);
  } else if (length < 0) {
    stringstream a("## HYT223 READOUT Error length = " + to_string(length));
    fLOG(WARNING, a.str());
    stringstream b;
    if (length == PI_BAD_HANDLE) b << " return value = PI_BAD_HANDLE";
    if (length == PI_BAD_PARAM) b << " return value = PI_BAD_PARAM";
    if (length == PI_I2C_READ_FAILED) b << " return value = PI_I2C_READ_FAILED";
    fLOG(WARNING, b.str());
  } else {
    if (fVerbose > 1) fLOG(ERROR, "#### readHYT223 readout error, length = " + to_string(length));
  }
   
  // -- trigger next measurement
  i2c_write_quick(fPiGPIO, handle, 0);
  i2c_close(fPiGPIO, handle);
#endif
}

// ----------------------------------------------------------------------
bool driveHardware::recoverI2CBus() {
#ifdef PI
  // TODO: verify pin numbers for your hardware.
  // -- GPIO00/GPIO01 correspond to physical header pins 27/28 on Raspberry Pi.
  const unsigned int SDA_PIN = 0; // pin 27: SDA0
  const unsigned int SCL_PIN = 1; // pin 28: SCL0
  const int pulseDelayUs = 10;

  fLOG(WARNING, "recoverI2CBus() start");

  set_mode(fPiGPIO, SDA_PIN, PI_INPUT);
  set_mode(fPiGPIO, SCL_PIN, PI_INPUT);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  int sdaBefore = gpio_read(fPiGPIO, SDA_PIN);
  int sclBefore = gpio_read(fPiGPIO, SCL_PIN);
  fLOG(INFO, "recoverI2CBus() levels before pulse: SDA="
       + to_string(sdaBefore) + " SCL=" + to_string(sclBefore));

  // -- if SDA is held low by a slave, release with up to 9 SCL pulses
  for (int i = 0; i < 9; ++i) {
    if (1 == gpio_read(fPiGPIO, SDA_PIN)) break;
    set_mode(fPiGPIO, SCL_PIN, PI_OUTPUT);
    gpio_write(fPiGPIO, SCL_PIN, 0);
    std::this_thread::sleep_for(std::chrono::microseconds(pulseDelayUs));
    set_mode(fPiGPIO, SCL_PIN, PI_INPUT); // release high
    std::this_thread::sleep_for(std::chrono::microseconds(pulseDelayUs));
  }

  // -- issue STOP condition: SDA low while SCL high, then SDA high
  set_mode(fPiGPIO, SDA_PIN, PI_OUTPUT);
  gpio_write(fPiGPIO, SDA_PIN, 0);
  std::this_thread::sleep_for(std::chrono::microseconds(pulseDelayUs));
  set_mode(fPiGPIO, SCL_PIN, PI_INPUT);
  std::this_thread::sleep_for(std::chrono::microseconds(pulseDelayUs));
  set_mode(fPiGPIO, SDA_PIN, PI_INPUT);
  std::this_thread::sleep_for(std::chrono::microseconds(pulseDelayUs));

  // -- restore I2C alternate function
  set_mode(fPiGPIO, SDA_PIN, PI_ALT0);
  set_mode(fPiGPIO, SCL_PIN, PI_ALT0);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  int sdaAfter = gpio_read(fPiGPIO, SDA_PIN);
  int sclAfter = gpio_read(fPiGPIO, SCL_PIN);
  fLOG(INFO, "recoverI2CBus() levels after pulse: SDA="
       + to_string(sdaAfter) + " SCL=" + to_string(sclAfter));
  bool ok = ((1 == sdaAfter) && (1 == sclAfter));
  fLOG(ok ? INFO : WARNING, string("recoverI2CBus() ") + (ok ? "success" : "incomplete (SDA/SCL still low)"));
  return ok;
#else
  return false;
#endif
}


// ----------------------------------------------------------------------
void driveHardware::doReconditioning() {
  double targetTemp(90.);
  int    targetTime(600);
  if (0 == fReconditioning) {
    fLOG(INFO, "Reconditioning: starting reconditioning");
    heatHYT223(true);
    turnOffValve(1);
    turnOffValve(0);
    fReconditioning = 1;
    fReconditioningWaitTime = 0;
  } 

  if (1 == fReconditioning) {
    if (fHYT223Temp > targetTemp) {
      fReconditioning = 2;
      stringstream a;
      a << "Reconditioning: temperature > " << fixed << setprecision(1) << targetTemp << "degC, starting reconditioning, wait time = " << fReconditioningWaitTime;
      fLOG(INFO, a.str());
    } else {
      stringstream a;
      a << "Reconditioning: temperature = " << fixed << setprecision(1) << fHYT223Temp << "degC, waiting for temperature to reach " << targetTemp;
      fLOG(INFO, a.str());
    }
  }

  if (2 == fReconditioning) {
    ++fReconditioningWaitTime;
    stringstream a;
    a << "Reconditioning: temperature = " << fixed << setprecision(1) << fHYT223Temp << "degC, wait time = " << fReconditioningWaitTime;
    fLOG(INFO, a.str());
    if (fReconditioningWaitTime > targetTime) {
      fReconditioning = 0;
      stringstream a;
      a << "Reconditioning: wait time > " << targetTime << " seconds, stopping with reconditioning, cool-down started";
      fLOG(INFO, a.str());
      heatHYT223(false);
      // -- flush+rinse to speed up cool-down
      turnOnValve(1);
      turnOnValve(0);
      fReconditioningWaitTime = 0;
    }
  }

}

// ----------------------------------------------------------------------
void driveHardware::heatHYT223(bool on) {
#ifdef PI
  stringstream a("heatHYT223: " + to_string(on));
  fLOG(INFO, a.str());
  
  int handle = i2c_open(fPiGPIO, I2CBUS, I2C_HEATHYT223_ADDR, 0);

  char command[2];
  command[0] = 0x04; // Port P_4! (The only port without dual use)
  if (on) {
    // -- Port low -> pFET passes VDD to heater
    command[1] = 0x00;
    fHeaterStatus = HEATER_MAX_STATUS;
  } else {
    // -- Port low -> pFET block VDD to heater
    command[1] = 0x7f;
    --fHeaterStatus;
  }

  for (int i = 0; i < 4; ++i) {
    fHYT223Data[i] = 0;
  }

  char addr;
  int length;
  addr = 0x27;
  length = i2c_write_device(fPiGPIO, handle, &addr, 1);
  length = i2c_read_device(fPiGPIO, handle, fHYT223Data, 1);
  for (unsigned int i = 0; i < length; ++i) {
    cout << dec << "config27 i = " << i << ": 0x" << hex
         << static_cast<int>(fHYT223Data[i])
         << dec
         << endl;
  }

  int result = i2c_write_device(fPiGPIO, handle, command, 2);

  for (int i = 0; i < 4; ++i) {
    fHYT223Data[i] = 0;
  }

  addr = command[0];
  length = i2c_write_device(fPiGPIO, handle, &addr, 1);
  length = i2c_read_device(fPiGPIO, handle, fHYT223Data, 1);
  
  i2c_close(fPiGPIO, handle);
#endif
}


// ----------------------------------------------------------------------
void driveHardware::readSHT85() {
  return;
#ifdef PI
  static int badReadoutCounter(0);
  static int readoutCounter(0);
  int length(0), cnt(0);

  ++readoutCounter;
  if (readoutCounter > 100) {
    stringstream a("periodic hard reset of SHT85");
    fLOG(INFO, a.str()); 
    gpio_write(fPiGPIO, GPIOPSUEN, 0);
    std::this_thread::sleep_for(fMilli100);
    std::this_thread::sleep_for(fMilli100);
    gpio_write(fPiGPIO, GPIOPSUEN, 1);
    readoutCounter = 0; 
  }
  
  int handle = i2c_open(fPiGPIO, I2CBUS, I2C_SHT85_ADDR, 0);

  // -- send a softreset (inspired by sensirion driver code)
  char softReset[2]; softReset[0] = 0xA2;  softReset[1] = 0x30;
  length = i2c_write_device(fPiGPIO, handle, softReset, 2);
  std::this_thread::sleep_for(fMilli20);

  int result = i2c_write_device(fPiGPIO, handle, fSHT85Config, 2);
  std::this_thread::sleep_for(fMilli20);

  length = i2c_read_device(fPiGPIO, handle, fSHT85Data, 6);
  if (length == PI_I2C_READ_FAILED) {
    i2c_close(fPiGPIO, handle);
    recoverI2CBus();
    handle = i2c_open(fPiGPIO, I2CBUS, I2C_SHT85_ADDR, 0);
    length = i2c_read_device(fPiGPIO, handle, fSHT85Data, 6);
  }
  while (cnt < 5) {
    if (6 == length) break;
    std::this_thread::sleep_for(fMilli20);
    length = i2c_read_device(fPiGPIO, handle, fSHT85Data, 6);
    ++cnt;
  }

  i2c_close(fPiGPIO, handle);

  if (length != 6) {
    fLOG(WARNING, "I2C Error: Input/output Error with SHT85, length = " + to_string(length)
         + " after " + to_string(cnt) + " read attempts");
    fI2CErrorOld = fI2CErrorCounter;
    ++fI2CErrorCounter;
  } else {
    fI2CErrorOld = fI2CErrorCounter;
    // -- convert the data
    //double cTemp = (((fSHT85Data[0] * 256) + fSHT85Data[1]) * 175.0) / 65535.0  - 45.0;
    //double humidity = (((fSHT85Data[3] * 256) + fSHT85Data[4])) * 100.0 / 65535.0;
    double norm     = 65535.0;
    double st       = (fSHT85Data[0]<<8) + fSHT85Data[1];
    double tmpTemp  = (st * 175.0) / norm  - 45.0;
    uint8_t crcRead = crc(&fSHT85Data[0], 2);
    if (crcRead != fSHT85Data[2]) {
      stringstream a("CRC check failed, ignoring read temperature  " + to_string(tmpTemp));
      fLOG(WARNING, a.str());
    } else if ((fSHT85Data[0] == 0) && (fSHT85Data[1] == 0)) {
      stringstream a("fSHT85Data[0,1] == 0,0 -- ignoring temperature reading ");
      fLOG(WARNING, a.str());
      ++badReadoutCounter;
    } else {
      fSHT85Temp      = tmpTemp;
      st              = (fSHT85Data[3]<<8) + fSHT85Data[4];
      double tmpRH    = (st * 100.0) / norm;
      uint8_t crcRead = crc(&fSHT85Data[3], 2);
      if (crcRead != fSHT85Data[5]) {
        stringstream a("CRC check failed, ignoring read RH  " + to_string(tmpRH));
        fLOG(WARNING, a.str());
      } else {
        fSHT85RH  = tmpRH;
      }
      fSHT85DP    = calcDP(fSHT85Temp, fSHT85RH, 1);
    }

    if (badReadoutCounter > 10) {
      stringstream a("badReadoutCounter > 10, invoking hard reset of SHT85");
      gpio_write(fPiGPIO, GPIOPSUEN, 0);
      std::this_thread::sleep_for(fMilli100);
      gpio_write(fPiGPIO, GPIOPSUEN, 1);
      badReadoutCounter = 0; 
    }

  }
#endif
}


// ----------------------------------------------------------------------
void driveHardware::readFlowmeter() {
#ifdef PI
  if (fVerbose > 0) fLOG(INFO, "readFlowmeter entered");
  int flowMeterStatus(0);
  int handle = i2c_open(fPiGPIO, I2CBUS, I2C_FLOWMETER_ADDR, 0);
  // -- set command byte to 0x0 (Register: Input Port, Protocol: Read Byte)
  char command = 0x0;
  int  length = i2c_write_device(fPiGPIO, handle, &command, 1);
  std::this_thread::sleep_for(fMilli20);

  char data = 0x0;
  length = i2c_read_device(fPiGPIO, handle, &data, 1);
  if (length == PI_I2C_READ_FAILED) {
    i2c_close(fPiGPIO, handle);
    recoverI2CBus();
    handle = i2c_open(fPiGPIO, I2CBUS, I2C_FLOWMETER_ADDR, 0);
    length = i2c_write_device(fPiGPIO, handle, &command, 1);
    if (length >= 0) {
      std::this_thread::sleep_for(fMilli20);
      length = i2c_read_device(fPiGPIO, handle, &data, 1);
    }
  }
  i2c_close(fPiGPIO, handle);

  if (length < 1) {
    flowMeterStatus = -1;
  } else {
    data = ~data;
    flowMeterStatus = data & 1;
  }
  if (0 == flowMeterStatus) {
    ++fBadFlowMeterReading;
    stringstream a("fBadFlowMeterReading = " + to_string(fBadFlowMeterReading));
    fLOG(WARNING, a.str());
  } else if (-1 == flowMeterStatus) {
    fFlowMeterStatus = -1;
  } else {
    fFlowMeterStatus = 1;
    fBadFlowMeterReading = 0;
  } 

  if (fBadFlowMeterReading > 10) {
    stringstream a("fBadFlowMeterReading > 10, changing fFlowMeterStatus = 0;");
    fFlowMeterStatus = 0;
    fLOG(WARNING, a.str());
  }
  
  if (fVerbose > 0) {
   stringstream a("flowmeter readout data =  " + to_string(data)
                    + " fFlowMeterStatus = " + to_string(fFlowMeterStatus));
    fLOG(INFO, a.str());
  }
#endif
}


#ifdef UZH
// ----------------------------------------------------------------------
int driveHardware::readI2C() {
  int r(1);
#ifdef PI
  
  int handle = i2c_open(fPiGPIO, I2CBUS, I2C_PCA_ADDR, 0);
  
  r = i2c_read_byte_data(fPiGPIO, handle, 0);
  
  r = r xor 255;
  string s_i2c = "LED is ";
  
  if (r & 2) s_i2c += "RED";
  else s_i2c += "GREEN";
  s_i2c += ", flow is ";
  if (r & 1) s_i2c += "on";
  else s_i2c += "off";
  
  //fLOG(WARNING, s_i2c+"  "+to_string(r));
  std::this_thread::sleep_for(fMilli20);
  i2c_close(fPiGPIO, handle);
#endif
return r;

}

#endif


// ----------------------------------------------------------------------
void driveHardware::readVProbe(int pos) {
  static map<int, int> nReads({{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}});
  fVprobeVoltages = "init";
  fVprobeGndVoltages = "init";
  static map<int, time_t> sLastVprobeReadout;
  static int first(1);
  if (first) {
    for (int i = 1; i <= 8; ++i) {
      sLastVprobeReadout[i] = time(NULL);
    }
    first = 0;
  }
  time_t now = time(NULL);
  const time_t vprobeRecoveryCutSeconds = 12*60*60; // 12h
  if (now - sLastVprobeReadout[pos] > vprobeRecoveryCutSeconds) {
    stringstream a("readVProbe(" + to_string(pos) + ") timeout (>12h), last readout was at "
                   + to_string(sLastVprobeReadout[pos])
                   + ", now = " + to_string(now));
    fLOG(WARNING, a.str());
    powerCycle3V3();
    sLastVprobeReadout[pos] = now;
  }

  ++nReads[pos];
  if (nReads[pos] > 100) {
    nReads[pos] = 0;
    fLOG(INFO, "readVProbe(" + to_string(pos) + ") reset due to too many reads");
    powerCycle3V3(1);
    std::this_thread::sleep_for(fMilli100);
    fLOG(INFO, "readVProbe(" + to_string(pos) + ") reset done");
  }

  fLOG(INFO, "readVProbe(" + to_string(pos) + ") start");
  static deque<string> sRecentVprobeRawReadouts;

  double VDD(3.3114);
  // -- TP corr. Doc:  6  5  9  10  11  7  8  12  4  3  2  13  14  26  8  1
  // -- TP wrong Doc: 12  8  7  11  10  9  5   6  1  8 26  14  13   2  3  4
  // -- Cnt:           0  1  2   3   4  5  6   7  8  9 10  11  12  13 14 15
  // int order[]  =   {6, 5, 9, 10, 11, 7, 8, 12, 4, 3, 2, 13, 14, 26, 8, 1};
  map<int, int> ord = {{6, 0}, {5, 1}, {9, 2},  {10, 3},  {11, 4},  {7, 5},   {8, 6},  {12, 7},
                       {4, 8}, {3, 9}, {2, 10}, {13, 11}, {14, 12}, {26, 13}, {1, 15}};
  char bufferC0[18] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x65};
  char bufferC1[18] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x65};

  char *buffer(0);

  double v[16] = {0};

  int ipos = pos - 1;
  int addresses[] = {((0x3<<4) | (2*ipos)), ((0x3<<4) | (2*ipos+1))};

  for (int iaddr = 0; iaddr < 2; ++iaddr) {
    if (0 == iaddr)
      buffer = bufferC0;
    else
      buffer = bufferC1;

    int lengthExp(18); // A = 10, B = 11, C = 12, D = 13, E = 14, F = 15
    int handle = i2c_open(fPiGPIO, I2CBUS, addresses[iaddr], 0);
    int length = i2c_read_device(fPiGPIO, handle, (iaddr == 0? bufferC0 : bufferC1), lengthExp);
    i2c_close(fPiGPIO, handle);

    stringstream raw;
    raw << "vprobe" << pos
        << " iaddr=" << iaddr
        << " addr(dec)=" << addresses[iaddr]
        << " addr(hex)=0x" << hex << addresses[iaddr] << dec
        << " length=" << length
        << " bytes=";
    for (int ibyte = 0; ibyte < lengthExp; ++ibyte) {
      if (ibyte > 0) raw << " ";
      raw << std::setfill('0') << std::setw(2) << hex
          << (static_cast<unsigned int>(static_cast<unsigned char>(buffer[ibyte])) & 0xff);

    }
    sRecentVprobeRawReadouts.push_back(raw.str());
    while (sRecentVprobeRawReadouts.size() > 10) sRecentVprobeRawReadouts.pop_front();

    // -- Check for valid second-to-last and last byte values
    bool badReadout(false);
    if (length == lengthExp) {
      unsigned int w16 = (static_cast<unsigned int>(static_cast<unsigned char>(buffer[16])) & 0xff);
      unsigned int w17 = (static_cast<unsigned int>(static_cast<unsigned char>(buffer[17])) & 0xff);
      if (w16 == 0xc9 && w17 == 0x01) {
        badReadout = false;
      } else {
        badReadout = true;
      }
      if (badReadout) {
        fLOG(ERROR, "Caught bad readout from the VProbe at i2c bus address " 
          + to_string(addresses[iaddr])  
          + " length = " + to_string(length) + " for position " + to_string(pos)
        );
        fLOG(ERROR, "Bad readout: w16 = " + to_string(w16) + ", w17 = " + to_string(w17));
        fLOG(ERROR, "Last VProbe raw readouts (oldest -> newest):");
        for (auto const &entry: sRecentVprobeRawReadouts) {
          fLOG(ERROR, "  " + entry);
        }
        fLOG(ERROR, "setting fVerbose to 10");
        fVerbose = 10;
        fLOG(ERROR, "power cycling 3.3V due to VProbe read error");
        powerCycle3V3(1);
        std::this_thread::sleep_for(fMilli100);
        fLOG(ERROR, "power cycling 3.3V done");

        // fLOG(ERROR, "closing I2C bus");
        // i2c_close(fPiGPIO, handle);
        // fLOG(ERROR, "recovering I2C bus");
        // recoverI2CBus();
        // fLOG(ERROR, "recovering I2C bus done");
  
        stringstream output;
        output <<  "vprobe" << pos << " = -999";
        fVprobeVoltages = output.str();

        stringstream output2;
        output2 <<  "vprobegnd = -999";
        fVprobeGndVoltages = output2.str();
        fMapVprobeGndVoltages.clear();
        fMapVprobeGndVoltages["gnd3"] = -999;
        fMapVprobeGndVoltages["gnd6"] = -999;
        fMapVprobeGndVoltages["gnd11"] = -999;
        fMapVprobeGndVoltages["gnd14"] = -999;
        fMapVprobeGndVoltages["gnd26"] = -999;

        // fLOG(ERROR, "power cycling 3.3V due to VProbe read error");
        // powerCycle3V3();
        // fLOG(ERROR, "power cycling 3.3V done");
        fLOG(ERROR, "returning due to bad readout");
        return;
      } else {
        if (10 == fVerbose) {
          fVerbose = 0;
          fLOG(INFO, "resetting fVerbose = 0");
        } 
        for (int i = 0; i < 8; ++i) {
          v[iaddr*8+i] = static_cast<unsigned int>(buffer[2*i] + (buffer[2*i+1]<<8))*VDD/65536;
        }
      }
    } else {
      fLOG(ERROR, "Failed to read from the VProbe at i2c bus address " 
           + to_string(addresses[iaddr])  
           + " length = " + to_string(length) + ""
          );
      if (length < 0) {
        stringstream b;
        if (length == PI_BAD_HANDLE) b << " return value = PI_BAD_HANDLE";
        if (length == PI_BAD_PARAM) b << " return value = PI_BAD_PARAM";
        if (length == PI_I2C_READ_FAILED) b << " return value = PI_I2C_READ_FAILED";
        if (b.str().size() > 0) {
          fLOG(ERROR, b.str());
        }
      }
      fLOG(ERROR, "Last VProbe raw readouts (oldest -> newest):");
      for (auto const &entry: sRecentVprobeRawReadouts) {
        fLOG(ERROR, "  " + entry);
      }
      stringstream output;
      output <<  "vprobe" << pos << " = -999";
      fVprobeVoltages = output.str();
      fMapVprobeGndVoltages.clear();
      fMapVprobeGndVoltages["gnd3"] = -999;
      fMapVprobeGndVoltages["gnd6"] = -999;
      fMapVprobeGndVoltages["gnd11"] = -999;
      fMapVprobeGndVoltages["gnd14"] = -999;
      fMapVprobeGndVoltages["gnd26"] = -999;

      // fLOG(ERROR, "power cycling 3.3V due to VProbe read error");
      // powerCycle3V3();
      // fLOG(ERROR, "power cycling 3.3V done");
      fLOG(ERROR, "returning due to failed readout");
      return;

      // stringstream a("power cycling 3.3V due to VProbe read error");
      // fLOG(ERROR, a.str());
      // powerCycle3V3();
      // stringstream b("power cycling 3.3V done");
      // fLOG(ERROR, b.str());
    }
  }

  if (fVerbose > 9) {
    fLOG(INFO, "bufferC0: ");
    for (int i = 0; i < 18; i +=2) {
      fLOG(INFO, "i = " + to_string(i) + ": " + formatHex(static_cast<unsigned int>(bufferC0[i])) + " " + formatHex(static_cast<unsigned int>(bufferC0[i+1])));
    }
    fLOG(INFO, "");
    fLOG(INFO, "bufferC1: ");
    for (int i = 0; i < 18; i +=2) {
      fLOG(INFO, "i = " + to_string(i) + ": " + formatHex(static_cast<unsigned int>(bufferC1[i])) + " " + formatHex(static_cast<unsigned int>(bufferC1[i+1])));
    }
    fLOG(INFO, "");
  }

  double vin   = v[ord[7]]  - v[ord[26]];
  double voffs = v[ord[8]]  - 0.25*(v[ord[14]] + v[ord[11]] + v[ord[6]] + v[ord[3]]);
  double vdda0 = v[ord[13]] - v[ord[14]];
  double vddd0 = v[ord[12]] - v[ord[14]];
  double vdda1 = v[ord[10]] - v[ord[11]];
  double vddd1 = v[ord[9]]  - v[ord[11]];
  double vdda2 = v[ord[5]]  - v[ord[6]];
  double vddd2 = v[ord[4]]  - v[ord[6]];
  double vdda3 = v[ord[2]]  - v[ord[3]];
  double vddd3 = v[ord[1]]  - v[ord[3]];

  fMapVprobeGndVoltages.clear();
  fMapVprobeGndVoltages["gnd3"] = v[ord[3]];
  fMapVprobeGndVoltages["gnd6"] = v[ord[6]];
  fMapVprobeGndVoltages["gnd11"] = v[ord[11]];
  fMapVprobeGndVoltages["gnd14"] = v[ord[14]];
  fMapVprobeGndVoltages["gnd26"] = v[ord[26]];

  stringstream output;
  output << "vprobe" << pos << " = " <<  std::setprecision(5)
         << vin << ","
         << voffs << ","
         << vdda0 << "," << vddd0 << ","
         << vdda1 << "," << vddd1 << ","
         << vdda2 << "," << vddd2 << ","
         << vdda3 << "," << vddd3 ;

  fVprobeVoltages = output.str();
  fLOG(INFO, "fVprobeVoltages = " + fVprobeVoltages);
}


// ----------------------------------------------------------------------
void driveHardware::readVProbeGnd() {
  stringstream output;
  output << "vprobegnd = " <<  std::setprecision(5)
         << fMapVprobeGndVoltages["gnd3"] << ","
         << fMapVprobeGndVoltages["gnd6"] << ","
         << fMapVprobeGndVoltages["gnd11"] << ","
         << fMapVprobeGndVoltages["gnd14"] << ","
         << fMapVprobeGndVoltages["gnd26"];

  fVprobeGndVoltages = output.str();
  cout << fVprobeGndVoltages << endl;
}

// ----------------------------------------------------------------------
float driveHardware::getTemperature() {
  return fAirTemp;
}


// ----------------------------------------------------------------------
float driveHardware::getRH() {
  return fAirRH;
}


// ----------------------------------------------------------------------
float driveHardware::getDP() {
  return fAirDP;
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
float driveHardware::calcDP(float temp, float rh, int mode) {
  double dp(0.);
  if (0 == mode) {
    dp = temp - (100. - rh)/5.; // most simple approximation
  } else if (1 == mode) {
    double A1(17.625), B1(243.04);
    double td0 = B1 * (log(rh/100.) + (A1*temp)/(B1 + temp));
    double td1 = A1 - log(rh/100.) - (A1*temp)/(B1 + temp);
    dp = td0/td1;

  }

  return static_cast<float>(dp);
}


// ----------------------------------------------------------------------
// https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c
char driveHardware::crc(char *data, size_t len) {
  char crc = 0xff;
  size_t i, j;
  for (i = 0; i < len; i++) {
    crc ^= data[i];
    for (j = 0; j < 8; j++) {
      if ((crc & 0x80) != 0)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }
  return crc;
}


// ----------------------------------------------------------------------
void driveHardware::lighting(int imode) {
#ifdef PI
  if (1 == imode) {
    gpio_write(fPiGPIO, GPIOGREEN, 0);
    gpio_write(fPiGPIO, GPIOYELLO, 0);
    gpio_write(fPiGPIO, GPIORED,   0);

    for (int i = 0; i < 2; ++i) {
      gpio_write(fPiGPIO, GPIOGREEN, 1);
      std::this_thread::sleep_for(2*fMilli100);
      gpio_write(fPiGPIO, GPIOYELLO, 1);
      std::this_thread::sleep_for(2*fMilli100);
      gpio_write(fPiGPIO, GPIORED, 1);
      std::this_thread::sleep_for(2*fMilli100);
      gpio_write(fPiGPIO, GPIORED, 0);
      std::this_thread::sleep_for(2*fMilli100);
      gpio_write(fPiGPIO, GPIOYELLO, 0);
      std::this_thread::sleep_for(2*fMilli100);
      gpio_write(fPiGPIO, GPIOGREEN, 0);
      std::this_thread::sleep_for(2*fMilli100);
    }
  }
#endif
}


// ----------------------------------------------------------------------
void  driveHardware::checkLid() {
  // -- keep reading from CAN bus in readAllParamsFromCANPublic() to minimize CAN errors
  fLidReading =  fTECData[1].reg["Temp_W"].value;
  if (fLidReading < 4100.) {
    // -- lid is locked
    fLidStatus = 1;
  } else if (fLidReading > 4100.) {
    // -- lid is open
    fLidStatus = -1;
  } else {
    // -- lid is closed, but not locked
    // -- unreachable, should not impact program logic above
    fLidStatus = 0;
  }
}


// ----------------------------------------------------------------------
void driveHardware::checkDiskspace() {
  struct statvfs diskData;
  statvfs(".", &diskData);
  double available = diskData.f_bavail * diskData.f_frsize;
  fFreeDiskspace = static_cast<int>(available/(1000.*1000.*1000.));
  fLOG(INFO, "Free Space : " + to_string(fFreeDiskspace));

  rsstools rss;
  stringstream a;
  a << "getCurrentRSS() = " << rss.getCurrentRSS();
  fLOG(INFO, a.str());
}


// ----------------------------------------------------------------------
void driveHardware::resetInterlock() {
  fInterlockStatus = 1;
  fStopOperations = 0;
  fVerbose = 0; 
#ifdef PI
  turnOnLV();
  // -- delay turning on HV
  std::chrono::milliseconds delay = std::chrono::milliseconds(1000);
  std::this_thread::sleep_for(delay);
  gpio_write(fPiGPIO, GPIOINT, 1);

  // -- indicate status on traffic light
  gpio_write(fPiGPIO, GPIORED,   0);
  fTrafficRed = 0;
  gpio_write(fPiGPIO, GPIOGREEN, 1);
  fTrafficGreen = 1; 
  gpio_write(fPiGPIO, GPIOYELLO, 0);
  fTrafficYellow = 0; 
#endif
  
  stringstream b;
  b << "Changed Interlock to HIGH";
  fLOG(INFO, b.str());
  emit signalSendToServer(QString::fromStdString(b.str()));

  fStatusString = "interlock reset";
}


// ----------------------------------------------------------------------
void driveHardware::breakInterlock() {
  fInterlockStatus = 0;
  fStopOperations = 1;
  fThrottleStatus = 0;
  
#ifdef PI
  gpio_write(fPiGPIO, GPIOINT, 0);

  // -- delay shutting off LV
  std::chrono::milliseconds delay = std::chrono::milliseconds(1000);
  std::this_thread::sleep_for(delay);
  turnOffLV();

  // -- indicate status on traffic light
  gpio_write(fPiGPIO, GPIORED,   0);
  fTrafficRed = 0;
  gpio_write(fPiGPIO, GPIOGREEN, 0);
  fTrafficGreen = 0; 
  gpio_write(fPiGPIO, GPIOYELLO, 0);
  fTrafficYellow = 0; 
#endif

  stringstream b;
  b << "Changed Interlocks to LOW";
  fLOG(INFO, b.str());
  emit signalSendToServer(QString::fromStdString(b.str()));
  
  
  fStatusString = "bad interlock";

}


// ----------------------------------------------------------------------
void driveHardware::throttleN2() {
  // -- the magic number is 5% relative humidity. The SHT85 does not like operation below that. 
  if (fAirRH < 8.5) {
    turnOffValve(0); 
    turnOffValve(1); 
    //fLOG(INFO, "throttleN2 turn off both valves, RH = " + to_string(fAirRH));
  } else if ((8.5 < fAirRH) && (fAirRH < 10.0)) {
    turnOffValve(0); 
    turnOnValve(1); 
    //fLOG(INFO, "throttleN2 turn off(on) valves 0(1), RH = " + to_string(fAirRH));
  } else if (fAirRH > 12.0) {
    turnOnValve(0); 
    turnOnValve(1); 
    //fLOG(INFO, "throttleN2 turn on both valves, RH = " + to_string(fAirRH));
  } else {
    //fLOG(INFO, "throttleN2 do nothing, RH = " + to_string(fAirRH));
  }
}
