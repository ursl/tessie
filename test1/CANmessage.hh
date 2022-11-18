#ifndef CANMESSAGE_HH
#define CANMESSAGE_HH

#include <vector>

#include "canFrame.hh"

class CANmessage {
public:
  CANmessage();

  // number of frames held
  unsigned int nFrames();
  // clear frames
  void clearFrames() {fFrames.clear();}
  // print
  void dump();
  void addFrame(canFrame &x);
  // read float and erase frame
  float getFloat(unsigned int itec, unsigned int ireg);
  // read int and erase frame
  int getInt(unsigned int itec, unsigned int ireg);
  // check for FRAS message (0x42)
  int getFRASMessage();
  // check for alarm
  int getAlarm();

private:
  std::vector<canFrame> fFrames;
  std::vector<canFrame> fFRASFrames;

  int fErrorCounter;
};

#endif // CANMESSAGE_HH
