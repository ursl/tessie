#include "CANmessage.hh"
#include "canFrame.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
CANmessage::CANmessage() {
  fErrorCounter = 0;
  map<int, deque<canFrame>> amap;
  deque<canFrame> adeq;
  for (int ireg = 0; ireg <= NREG; ++ireg) {
    amap.insert(make_pair(ireg, adeq));
  }
  for (int itec = 1; itec <= 8; ++itec) {
    fMapFrames.insert(make_pair(itec, amap));
  }
}


// ----------------------------------------------------------------------
void CANmessage::addFrame(canFrame &x) {
  bool filled(false);

  if ((1 <= x.fTec) && (x.fTec <= 8) && (0 == x.fType)
      && (1 == x.fPrivate) && (1 == x.fShift)) {
    // -- cmd GetSWVersion 301
    cout << "DBX "; x.dump();
    fqFrames.push_front(x);
    filled = true;
  }

  // -- received a CAN READ message from a TEC
  if ((1 <= x.fTec) && (x.fTec <= 8) && (1 == x.fType)) {
    fMapFrames[x.fTec][x.fReg].push_front(x);
    filled = true;
  }

  if (!filled && (x.fFRAS > 0)) {
    fqFRASFrames.push_back(x);
    filled = true;
  }

  if (!filled && (x.fAlarm > 0)) {
    fqAlarmFrames.push_back(x);
    filled = true;
  }

  if (!filled && (0 <= x.fTec) && (x.fTec <= 8) && (2 == x.fType)) {
    // -- ignore writes?!?!
    filled = true;
  }

  if (!filled && (0 == x.fTec) && (1 == x.fType)
      && (0 == x.fPrivate) && (1 == x.fShift)) {
    // -- ignore read requests 210
    filled = true;
  }

  if (!filled && (0 == x.fTec) && (0 == x.fType)
      && (0 == x.fPrivate) && (1 == x.fShift)) {
    // -- ignore (public) TEC watchdog commands
    filled = true;
  }

  if (!filled && (0 == x.fTec) && (4 == ((x.fCanId & 0x40)>>4))
      && (0 == x.fPrivate) && (0 == x.fShift)) {
    // -- ignore FRAS entertainment 40
    filled = true;
  }

  if (!filled && (1 == x.fTec) && (4 == ((x.fCanId & 0x40)>>4))
      && (0 == x.fPrivate) && (0 == x.fShift)) {
    // -- ignore FRAS entertainment 41
    filled = true;
  }

  if (!filled) {
    string errstring = "did not fill "
        + x.getString()
        + " fPrivate = " + to_string(x.fPrivate)
        + " fShift = " + to_string(x.fShift)
        + " fType = " + to_string(x.fType)
        + " fTec = " + to_string(x.fTec)
        ;
    fqErrors.push_front(errstring);
  }
}


// ----------------------------------------------------------------------
void CANmessage::clearAllFrames() {
  if (0) {
    cout << "0 clearAllFrames: " << endl;
    printMapFramesSize();
  }
  for (auto &itt :fMapFrames) {
    for (auto &itr: itt.second) {
      itr.second.clear();
    }
  }
  if (0) {
    cout << "1 clearAllFrames: " << endl;
    printMapFramesSize();
  }

  fqErrors.clear();
}


// ----------------------------------------------------------------------
void CANmessage::dump() {
  cout << "dump map<int, map<int, deque>> containers" << endl;
  for (auto &itt :fMapFrames) {
    for (auto &itr: itt.second) {
      for (auto &itc: itr.second) {
        itc.dump();
      }
    }
  }
}


// ----------------------------------------------------------------------
void CANmessage::printMapFramesSize() {
  cout << dec;
  for (auto &itt :fMapFrames) {
    cout << " T" << itt.first << "(" << itt.second.size() << ") R: ";
    int idx(0);
    for (auto &itr: itt.second) {
      if (itr.second.size() > 0) {
        cout << idx << "(" << itr.second.size() << ") ";
      }
      ++idx;
    }
  }
  cout << endl;
}


// ----------------------------------------------------------------------
unsigned int CANmessage::nFrames() {
//  return fFrames.size();
  return 99;
}


// ----------------------------------------------------------------------
unsigned int CANmessage::nFrames(int itec) {
  return fMapFrames[itec].size();
}


// ----------------------------------------------------------------------
unsigned int CANmessage::nFrames(int itec, int ireg) {
  return fMapFrames[itec][ireg].size();
}

// ----------------------------------------------------------------------
int CANmessage::nErrors() {
  return fErrorCounter;
}


// ----------------------------------------------------------------------
deque<string> CANmessage::getErrors() {
  return fqErrors;
}


// ----------------------------------------------------------------------
float CANmessage::getFloat(unsigned int itec, unsigned int ireg) {
  float result(-999.);

  if (nFrames(itec, ireg) > 0) {
    result = fMapFrames[itec][ireg].front().fFloatVal;
    if (0) {
      cout << " pop_front "; fMapFrames[itec][ireg].front().dump(false); cout << endl;
    }
    fErrorFrame = fMapFrames[itec][ireg].front();
    fMapFrames[itec][ireg].pop_front();
  }

  if (result < -990.) {
    string errstring = "parse issue float: reg " + to_string(ireg)
      + " itec " + to_string(itec)
      + " getFloat " + to_string(result)
      + " canFrame: ->" + fErrorFrame.getString() + "<-";
    fqErrors.push_front(errstring);
    ++fErrorCounter;
  }
  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getInt(unsigned int itec, unsigned int ireg) {
  int result(-999);

  if (nFrames(itec, ireg) > 0) {
    result = fMapFrames[itec][ireg].front().fIntVal;
    if (0) {
      cout << " pop_front "; fMapFrames[itec][ireg].front().dump(false); cout << endl;
    }
    fErrorFrame = fMapFrames[itec][ireg].front();
    fMapFrames[itec][ireg].pop_front();
  }

  if (result < -990) {
    ++fErrorCounter;
    string errstring = "parse issue int: reg " + to_string(ireg)
       + " itec " + to_string(itec)
       + " getInt " + to_string(result);
    fqErrors.push_front(errstring);
  }
  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getFRASMessage() {
  int result = fqFRASFrames.size();
  // -- clear it
  //  fErrorFrame = fqFRASFrames.front();
  if (result > 0) fqFRASFrames.pop_front();
  return result;
}


// ----------------------------------------------------------------------
canFrame CANmessage::getFrame() {
  canFrame result;
  if (fqFrames.size() > 0) {
    result = fqFrames.front();
    //    fErrorFrame = result;
    fqFrames.pop_front();
  }
  return result;
}

// ----------------------------------------------------------------------
int CANmessage::getAlarm() {
  int result(0);
  if (fqAlarmFrames.size() > 0) {
    result = fqAlarmFrames.front().fAlarm;
    //    fErrorFrame = fqAlarmFrames.front();
    fqAlarmFrames.pop_front();
  }
  return result;
}
