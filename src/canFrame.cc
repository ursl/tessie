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
  cout << "canFrame: canid = " << std::hex << canid << std::dec << " len = " << len << endl;

  fData.clear();
  for (int i = 0; i < len; ++i) fData.push_back(data[i]);

  fPrivate = (fCanId & 0x100)>>8;
  fShift   = (fCanId & 0x600)>>9;

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
    fType = (fCanId & 0x30)>>4;

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

    // -- non-data cases
    if (0 == fType) {
      if (4 == fReg) {
        // -- alarms
        if (fIntVal > 0) {
          fAlarm = fIntVal;
        } else {
          fAlarm = 0xdeadface;
        }
      } else if (6 == fReg) {
        // -- software version in one byte
        char ndata = fData[1];
        fIntVal = static_cast<int>(ndata);
      }
    }

  }
}


// ----------------------------------------------------------------------
string canFrame::getString() {
   stringstream sbla;
   sbla << std::hex << fCanId << " ["  << std::dec << fdlen << std::hex << "]";
   //   cout << "sbla ->" << sbla.str() << "<-"  << endl;
   char sbuffer[5];
   if (fdlen > 1 && fdlen < (0x1<<10)) {
     for (unsigned int i = 0; i < fdlen; ++i) {
       sprintf(sbuffer, " %02X", static_cast<int>(fData[i]));
       sbla << sbuffer;
     }
   } else {
     sbla << " bad format (len = " << fdlen << ")";
   }
   sbla << std::dec
        << ". tec = " << fTec
        << " reg = " << fReg
        << " val = " << fFloatVal << "/" << fIntVal;
   return sbla.str();
}


// ----------------------------------------------------------------------
void canFrame::dump(bool eol){
  cout << "canFrame::dump" << endl;
  string sbla = getString();
  cout << sbla;
  if (eol) std::cout << std::endl;
}
