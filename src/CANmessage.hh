#ifndef CANMESSAGE_HH
#define CANMESSAGE_HH

#include <string>
#include <vector>
#include <deque>
#include <map>

#include "canFrame.hh"

// CANmessage holds various deques (double-ended queue) for various types of canFrames
// (from FRAS, per TEC/register, errors)
//
// canFrame holds the raw CAN frame from the CAN bus
 

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
  void clearAllFrames();
  // -- get accumulated error messages
  std::deque<std::string> getErrors();

  // -- print
  void dump();
  void printMapFramesSize();

  void addFrameV(canFrame &x);
  void addFrame(canFrame &x);

  // -- read float and erase frame
  float getFloat(unsigned int itec, unsigned int ireg);
  // -- read int and erase frame
  int getInt(unsigned int itec, unsigned int ireg);
  // -- check for FRAS message (0x42), returns 1 (0) if there is one (none)
  int getFRASMessage();
  // -- check for alarm
  int getAlarm();
  // -- get a raw frame
  canFrame getFrame();
  // -- get the erroneous frame
  canFrame getErrorFrame() {return fErrorFrame;}

private:
  std::map<int, std::map<int, std::deque<canFrame>>> fMapFrames;
  std::deque<canFrame>                 fqFRASFrames;
  std::deque<canFrame>                 fqAlarmFrames;
  // -- all the other frames (in response to commands, error code, ...)
  std::deque<canFrame>                 fqFrames;

  int                      fErrorCounter;
  std::deque<std::string>  fqErrors;
  canFrame                 fErrorFrame;
};

#endif // CANMESSAGE_HH
