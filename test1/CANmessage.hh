#ifndef CANMESSAGE_HH
#define CANMESSAGE_HH

#include <vector>

#include "canFrame.hh"

class CANmessage {
public:
  CANmessage();

  unsigned int nFrames();
  void  dump();
  void  addFrame(canFrame &x);
  // read float and erase frame
  float getFloat(unsigned int itec, unsigned int ireg);
  // read int and erase frame
  int   getInt(unsigned int itec, unsigned int ireg);

private:
  std::vector<canFrame> fFrames;
};

#endif // CANMESSAGE_HH
