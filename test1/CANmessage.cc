#include "CANmessage.hh"
#include "canFrame.hh"

using namespace std;

// ----------------------------------------------------------------------
CANmessage::CANmessage() {

}

// ----------------------------------------------------------------------
void CANmessage::addFrame(canFrame &x) {
  cout << "  adding ";
  x.dump();
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
  float result(-99.);
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if ((tec == it->fTec) && (reg == it->fReg)) {
      result = it->fFloatVal;
      fFrames.erase(it);
      break;
    }
  }
  return result;
}

// ----------------------------------------------------------------------
int CANmessage::getInt(unsigned int tec, unsigned int reg) {
  int result(-99);
  for (std::vector<canFrame>::iterator it = fFrames.begin(); it != fFrames.end(); ++it) {
    if ((tec == it->fTec) && (reg == it->fReg)) {
      result = it->fFloatVal;
      fFrames.erase(it);
      break;
    }
  }
  return result;
}
