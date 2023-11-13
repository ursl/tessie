#include <iostream>

using namespace std;

int main() {
  unsigned char bufferC0[18] = {0x01, 0x02, 0x00, 0x08, 0x00, 0x07, 0x01, 0x01, 0x01, 0x00,
                                0x00, 0x09, 0x00, 0x05, 0x00, 0x06, 0xff, 0xff};
  unsigned char bufferC1[18] = {0x00, 0x01, 0x00, 0x08, 0x02, 0x06, 0x01, 0x04, 0x01, 0x03,
                                0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0xff, 0xff};
  unsigned char *buffer(0);

  cout << "bufferC0: ->" << hex << 0x12 << dec << "<-" << endl;
  cout << "bufferC0: ->" <<  static_cast<int>(bufferC0[0]) << "<-" << dec << endl;
  for (int i = 0; i < 18; i +=2) {
    cout << dec << "i = " << i << ": 0x" << hex
         << static_cast<int>(bufferC0[i])
         << static_cast<int>(bufferC0[i+1])
         << ". ";
  }
  cout << endl;
  cout << "bufferC1" << endl;
  for (int i = 0; i < 18; i +=2) {
    cout << dec << "i = " << i << ": 0x" << hex
         << static_cast<int>(bufferC0[i])
         << static_cast<int>(bufferC0[i+1])
         << dec
         << ". ";
  }
  cout << endl;

  float VDD(3.3114);
  unsigned int iaddr = 0; 
  double v[18] = {0.};
  buffer = bufferC0;
  for (int i = 0; i < 16; i += 2) {
    v[iaddr*i] = static_cast<double>((buffer[i] + (buffer[i+1]<<8)))/65536.*VDD;
    cout << "i = " << i/2 << ": buffer[] = "
         << hex
         << static_cast<int>(buffer[i] + (buffer[i+1]<<8)) << " -> " << v[iaddr*i]
         << dec << endl;
  }

  return 0;
}
