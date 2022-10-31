#include "CANmessage.hh"
#include "canFrame.hh"

using namespace std;

// ----------------------------------------------------------------------
CANmessage::CANmessage() {

}

// ----------------------------------------------------------------------
void CANmessage::addFrame(canFrame &x) {
  //  if (0) {
  //    cout << "  adding ";
  //    x.dump();
  //  }
  fFrames.push_back(x);
}


// ----------------------------------------------------------------------
void CANmessage::dump() {
  for (unsigned int i = 0; i < fFrames.size(); ++i) {
    fFrames[i].dump();
  }
}

// ----------------------------------------------------------------------
unsigned int CANmessage::nFrames() {
  return fFrames.size();
}


// ----------------------------------------------------------------------
float CANmessage::getFloat(unsigned int tec, unsigned int reg) {
  float result(-98.);
  // -- find the last one
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if ((tec == it->fTec) && (reg == it->fReg)) {
      result = it->fFloatVal;
      //fFrames.erase(it);
      //break;
    }
  }
  return result;
}

// ----------------------------------------------------------------------
int CANmessage::getInt(unsigned int tec, unsigned int reg) {
  int result(-98);
  // -- find the last one
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if ((tec == it->fTec) && (reg == it->fReg)) {
      result = it->fIntVal;
      //fFrames.erase(it);
      //break;
    }
  }
  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getFRASMessage() {
  int result(0);
  // -- find the last one
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if (it->fFRAS > 0) {
       result = it->fFRAS;
    }
  }

  return result;
}


// ----------------------------------------------------------------------
int CANmessage::getAlarm() {
  int result(0);
  // -- find the last one
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if (it->fAlarm > 0) {
       result = it->fAlarm;
    }
  }

  return result;
}
