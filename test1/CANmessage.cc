#include "CANmessage.hh"
#include "canFrame.hh"

using namespace std;

// ----------------------------------------------------------------------
CANmessage::CANmessage() {
  fErrorCounter = 0;
}

// ----------------------------------------------------------------------
void CANmessage::addFrame(canFrame &x) {
  //  if (0) {
  //    cout << "  adding ";
  //    x.dump();
  //  }
  if (0 == x.fFRAS) {
    fFrames.push_back(x);
  } else if (x.fAlarm > 0) {
    fqAlarmFrames.push_back(x);
  } else {
    fFRASFrames.push_back(x);
    fqFRASFrames.push_back(x);
  }
}


// ----------------------------------------------------------------------
void CANmessage::dump() {
  cout << "dump vector containers" << endl;
  for (unsigned int i = 0; i < fFrames.size(); ++i) {
    fFrames[i].dump();
  }

  cout << "dump map<int, map<int, deque>> containers" << endl;
  for (auto &itt :fMapFrames) {
    for (auto &itd: itt.second) {
      for (auto &itc: itd.second) {
        itc.dump();
      }
    }
  }
}


// ----------------------------------------------------------------------
unsigned int CANmessage::nFrames() {
  return fFrames.size();
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
int CANmessage::getIntV(unsigned int tec, unsigned int reg) {
  int result(-98);
  // -- find the last one
  for (vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if ((tec == it->fTec) && (reg == it->fReg)) {
      result = it->fIntVal;
      //fFrames.erase(it);
      //break;
    }
  }
  if (result < -90.) {
    ++fErrorCounter;
    cout << "Error: reg " << reg << " itec " << tec << " getInt " << result << endl;
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
int CANmessage::getFRASMessageV() {
  int result = fFRASFrames.size();
  if (result > 0) fFRASFrames.clear();
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
int CANmessage::getAlarmV() {
  int result(0);
  // -- find the last one
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if (it->fAlarm > 0) {
       result = it->fAlarm;
    }
  }

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
