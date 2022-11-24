#include "CANmessage.hh"
#include "canFrame.hh"

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
}


// ----------------------------------------------------------------------
void CANmessage::addFrame(canFrame &x) {
  bool filled(false);
  // -- received a CAN message from a TEC
  if (1 <= x.fTec && x.fTec <= 8) {
    fMapFrames[x.fTec][x.fReg].push_front(x);
    filled = true;
  } else {
    if (1 == x.fFRAS) {
      fqFRASFrames.push_back(x);
      filled = true;
    }
    if (x.fAlarm > 0) {
      fqAlarmFrames.push_back(x);
      filled = true;
    }
  }
  if (!filled) {
    if (0) {
      cout << "did NOT fill "; x.dump(false); cout << endl;
    }
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
float CANmessage::getFloat(unsigned int itec, unsigned int ireg) {
  float result(-98.);

  if (nFrames(itec, ireg) > 0) {
    result = fMapFrames[itec][ireg].front().fFloatVal;
    if (0) {
      cout << " pop_front "; fMapFrames[itec][ireg].front().dump(false); cout << endl;
    }
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
    if (0) {
      cout << " pop_front "; fMapFrames[itec][ireg].front().dump(false); cout << endl;
    }
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
