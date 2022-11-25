#ifndef CANFRAME_HH
#define CANFRAME_HH

#include <vector>
#include <string>

#define NREG 18

class canFrame {
public:
  canFrame() { };
  canFrame(int canID, int len, unsigned char *data);

  std::string getString();

  void dump(bool eol = true);

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
