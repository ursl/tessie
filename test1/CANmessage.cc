#include "CANmessage.hh"
#include "canFrame.hh"

using namespace std;

// ----------------------------------------------------------------------
CANmessage::CANmessage() {
  fErrorCounter = 0;
  for (int itec = 1; itec <= 8; ++itec) {
    for (int ireg = 0; ireg <= NREG; ++ireg) {
      map<int, deque<canFrame>> amap;
      deque<canFrame> adeq;
      amap.insert(make_pair(ireg, adeq));
      fMapFrames.insert(make_pair(itec, amap));
    }
  }
}


// ----------------------------------------------------------------------
void CANmessage::clearAllFrames() {
  for (auto &itt :fMapFrames) {
    for (auto &itr: itt.second) {
      itr.second.clear();
    }
  }
}


// ----------------------------------------------------------------------
void CANmessage::addFrame(canFrame &x) {
  if (0 == x.fFRAS) {
    if (1) {
      cout << "  adding ";
      x.dump();
    }
    fMapFrames[x.fTec][x.fReg].push_front(x);
  } else if (x.fAlarm > 0) {
    fqAlarmFrames.push_back(x);
  } else {
    fFRASFrames.push_back(x);
    fqFRASFrames.push_back(x);
  }
}


// ----------------------------------------------------------------------
void CANmessage::dump() {
//  cout << "dump vector containers" << endl;
//  for (unsigned int i = 0; i < fFrames.size(); ++i) {
//    fFrames[i].dump();
//  }

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
float CANmessage::getFloat(unsigned int itec, unsigned int ireg) {
  float result(-98.);

  if (nFrames(itec, ireg) > 0) {
    result = fMapFrames[itec][ireg].front().fFloatVal;
    cout << " pop_front "; fMapFrames[itec][ireg].front().dump(false); cout << endl;
    fMapFrames[itec][ireg].pop_front();
  }

  if (result < -90.) {
    ++fErrorCounter;
    cout << "Error: reg " << ireg << " itec " << itec << " getFloat " << result << endl;
  }
  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getInt(unsigned int itec, unsigned int ireg) {
  int result(-98);

  if (nFrames(itec, ireg) > 0) {
    result = fMapFrames[itec][ireg].front().fIntVal;
    cout << " pop_front "; fMapFrames[itec][ireg].front().dump(false); cout << endl;
    fMapFrames[itec][ireg].pop_front();
  }

  if (result < -90.) {
    ++fErrorCounter;
    cout << "Error: reg " << ireg << " itec " << itec << " getInt " << result << endl;
  }
  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getFRASMessage() {
  int result = fqFRASFrames.size();
  // -- clear it
  if (result > 0) fqFRASFrames.pop_front();
  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getAlarm() {
  int result(0);
  if (fqAlarmFrames.size() > 0) {
    result = fqAlarmFrames.front().fAlarm;
    fqAlarmFrames.pop_front();
  }
  return result;
}
