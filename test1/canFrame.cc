#include "canFrame.hh"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
canFrame::canFrame(int canid, int len, unsigned char *data) {
  fCanId = canid;
  fdlen  = len;
  for (int i = 0; i < len; ++i) fData.push_back(data[i]);

  if (ADDRESS_FRAS == fCanId) {
    // -- This is a FRAS message
    fFRAS = ADDRESS_FRAS;
    fAlarm = 0;
    fReg  = 0;
    fType = 0;
    fTec  = 0;
    fIntVal   = -99;
    fFloatVal = -99.;
  } else {
    fFRAS = 0;
    fAlarm = 0;
    fTec = fCanId & 0xf;
    fReg = data[0];
    fType = fCanId & 0x30;

    // -- parse data if present
    if (5 == fdlen) {
      char ndata[4];
      for (int i = 0; i < len; ++i) ndata[i] = fData[i+1];
       memcpy(&fIntVal, ndata, sizeof fIntVal);
       memcpy(&fFloatVal, ndata, sizeof fFloatVal);
     } else {
       fIntVal   = -99;
       fFloatVal = -99.;
     }

    // -- alarms
    if ((0 == fType) && (4 == fReg)) {
      if (fIntVal > 0) {
        fAlarm = fIntVal;
      } else {
        fAlarm = 0xdeadface;
      }
    }
  }
}


// ----------------------------------------------------------------------
string canFrame::getString() {
   stringstream sbla;
   sbla << std::hex << fCanId << " [" << fdlen << "]";
   char sbuffer[5];
   for (unsigned int i = 0; i < fdlen; ++i) {
       sprintf(sbuffer, " %02X", static_cast<int>(fData[i]));
       sbla << sbuffer;
     }
   sbla << std::dec
        << ". tec = " << fTec
        << " reg = " << fReg
        << " val = " << fFloatVal << "/" << fIntVal;
   return sbla.str();
}


// ----------------------------------------------------------------------
void canFrame::dump(bool eol){
  string sbla = getString();
  cout << sbla;
  if (eol) std::cout << std::endl;
}

