#ifndef CANFRAME_HH
#define CANFRAME_HH

#include <cstring>
#include <iostream>
#include <strstream>
#include <vector>

class canFrame {
public:
  canFrame() { };

  canFrame(int canid, int len, unsigned char *data) {
    fCanId = canid;
    fdlen  = len;
    for (int i = 0; i < len; ++i) fData.push_back(data[i]);

    if (0x42 == fCanId) {
      // -- This is a FRAS message
      fFRAS = 0x42;
      fReg  = 0;
      fType = 0;
      fTec  = 0;
      fIntVal   = -99;
      fFloatVal = -99.;
    } else {
      fFRAS = 0;

      fTec = fCanId & 0xf;
      fReg = data[0];
      fType = fCanId & 0x30;

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
        }
      }
    }
  }

  void dump(bool eol = true) {
    //std::strstream s;
    std::cout << std::hex << fCanId << " [" << fdlen << "] ";
    char sbuffer[5];
    for (unsigned int i = 0; i < fdlen; ++i) {
        sprintf(sbuffer, "%02X ", static_cast<int>(fData[i]));
        std::cout << sbuffer;
      }
    std::cout << std::dec
              << ". tec = " << fTec
              << " reg = " << fReg
              << " val = " << fFloatVal << "/" << fIntVal;
    if (eol) std::cout << std::endl;
  }

  // -- raw data
  unsigned int fCanId;
  unsigned int fdlen;

  // -- interpreted data
  unsigned int fAlarm;
  unsigned int fFRAS;
  unsigned int fTec;
  unsigned int fReg;

  unsigned int fType; // 0 CMD, 1 READ, 2 Write

  int   fIntVal;
  float fFloatVal;

  // -- vector with raw data (not just the 4 bytes with the float/int)
  std::vector<unsigned char> fData;
};

#endif // CANFRAME_HH
