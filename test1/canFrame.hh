#ifndef CANFRAME_HH
#define CANFRAME_HH

#include <cstring>
#include <iostream>
#include <strstream>
#include <vector>

class canFrame {
public:
  canFrame() { };

  canFrame(int canid, int len, char *data) {
    fCanId = canid;
    fdlen  = len;
    for (int i = 0; i < len; ++i) fData.push_back(data[i]);

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
  }

  void dump() {
    std::strstream s;
    s << fCanId << " [" << fdlen << "] ";
    for (unsigned int i = 0; i < fdlen; ++i) {
      s << fData[i] << " ";
    }
    s << ". tec = " << fTec
      << " reg = " << fReg
      << " val = " << fFloatVal << "/" << fIntVal;
    std::cout << s.str() << std::endl;
  }

  // -- raw data
  unsigned int fCanId;
  unsigned int fdlen;

  // -- interpreted data
  unsigned int fTec;
  unsigned int fReg;

  unsigned int fType; // 0 CMD, 1 READ, 2 Write

  int   fIntVal;
  float fFloatVal;

  // -- vector with raw data (not just the 4 bytes with the float/int)
  std::vector<char> fData;
};

#endif // CANFRAME_HH
