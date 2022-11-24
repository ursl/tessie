#ifndef CANMESSAGE_HH
#define CANMESSAGE_HH

#include <vector>
#include <deque>
#include <map>

#include "canFrame.hh"

class CANmessage {
public:
  CANmessage();

  // -- number of frames held
  unsigned int nFrames(); // vectors REMOVE
  unsigned int nFrames(int itec);
  unsigned int nFrames(int itec, int ireg);

  // -- get errors encountered
  int nErrors();
  // -- clear frames
//  void clearFrames() {fFrames.clear();} // vectors REMOVE
  void clearAllFrames();
  // -- print
  void dump();

  void addFrameV(canFrame &x);
  void addFrame(canFrame &x);

  // -- read float and erase frame
//  float getFloatV(unsigned int itec, unsigned int ireg);
  float getFloat(unsigned int itec, unsigned int ireg);
  // -- read int and erase frame
//  int getIntV(unsigned int itec, unsigned int ireg);
  int getInt(unsigned int itec, unsigned int ireg);
  // -- check for FRAS message (0x42), returns 1 (0) if there is one (none)
  int getFRASMessageV();
  int getFRASMessage();
  // -- check for alarm
  int getAlarmV();
  int getAlarm();

private:
//  std::vector<canFrame> fFrames;
  std::vector<canFrame> fFRASFrames;

  // map<tec, map<reg, deque<canFrame>>>
  std::map<int, std::map<int, std::deque<canFrame>>> fMapFrames;
  std::deque<canFrame>                 fqFRASFrames;
  std::deque<canFrame>                 fqAlarmFrames;

  int fErrorCounter;
};

#endif // CANMESSAGE_HH
