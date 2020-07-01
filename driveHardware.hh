#ifndef DRIVEHARDWARE_H
#define DRIVEHARDWARE_H


class driveHardware {
public:
  driveHardware(int freq = 1 /*sec*/, int offset = 0);

  void runPrintout();

  void setFrequency(int x);
  void setOffset(int x);
  int getFrequency();
  int getOffset();

private:
  int fFrequency;
  int fOffset;
};

#endif // DRIVEHARDWARE_H
