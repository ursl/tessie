#include "driveHardware.hh"
#include <iostream>
#include <unistd.h>

#include <chrono>
#include <thread>

using namespace std;

// ----------------------------------------------------------------------
driveHardware::driveHardware(int freq, int offset) : fFrequency(freq), fOffset(offset) {

}



// ----------------------------------------------------------------------
void driveHardware::runPrintout() {
  int cnt(fOffset);
  std::chrono::milliseconds sec(1000/fFrequency);
  while (1) {
    cout << "countUp: " << cnt++ << " fFrequency = " << fFrequency << endl;
    //    sleep(1./fFrequency);
    std::this_thread::sleep_for(sec);
  }
}

// ----------------------------------------------------------------------
void driveHardware::setFrequency(int freq) {
  fFrequency = freq;
}

// ----------------------------------------------------------------------
void driveHardware::setOffset(int oset) {
  fOffset = oset;
}

// ----------------------------------------------------------------------
int driveHardware::getFrequency() {
  return fFrequency;
}

// ----------------------------------------------------------------------
int driveHardware::getOffset() {
  return fOffset;
}
