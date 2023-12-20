#include <sstream>
#include <iostream>


// ----------------------------------------------------------------------
// -- first part is from Sensirion

//CRC
#define POLYNOMIAL 0x131 //P(x)=x^8+x^5+x^4+1 = 100110001

typedef enum{
  CHECKSUM_ERROR = 0x04
} etError;

typedef unsigned char u8t;

using namespace std;

//============================================================
u8t SMF3000_CheckCrc (u8t data[], u8t nbrOfBytes, u8t checksum)
//============================================================
//calculates checksum for n bytes of data
//and compares it with expected checksum
//input: data[] checksum is built based on this data
// nbrOfBytes checksum is built for n bytes of data
// checksum expected checksum
//return: error: CHECKSUM_ERROR = checksum does not match
// 0 = checksum matches
//============================================================
{
  u8t crc = 0;
  u8t byteCtr;
  //calculates 8-Bit checksum with given polynomial
  for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr) {
    crc ^= (data[byteCtr]);
    for (u8t bit = 8; bit > 0; --bit)  {
      if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else crc = (crc << 1);
    }
  }
  if (crc != checksum) return CHECKSUM_ERROR;
  else return 0;
}

//============================================================
u8t SMF3000_Crc (u8t data[], u8t nbrOfBytes)
//============================================================
//calculates checksum for n bytes of data
//input: data[] checksum is built based on this data
// nbrOfBytes checksum is built for n bytes of data
//return: checksum
//============================================================
{
  u8t crc = 0xff;
  u8t byteCtr;
  //calculates 8-Bit checksum with given polynomial
  for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr) {
    crc ^= (data[byteCtr]);
    for (u8t bit = 8; bit > 0; --bit)  {
      if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else crc = (crc << 1);
    }
  }
  return crc;
}
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c
char crc(char *data, size_t len) {
  char crc = 0xff;
  size_t i, j;
  for (i = 0; i < len; i++) {
    crc ^= data[i];
    for (j = 0; j < 8; j++) {
      if ((crc & 0x80) != 0)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }
  return crc;
}


// ----------------------------------------------------------------------
// ./crc -i 0xbeef
// number: 48879 0xbeef
// bytes: beef
// crc(0xbeef) = 92
// crc(0xbeef) = 92

int main(int argc, char *argv[]) {

  string sid;
  unsigned short id(0);

  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-i"))  {sid = string(argv[++i]); }
  } 

  std::stringstream ss;
  ss << std::hex << sid;
  ss >> id; 

  char data[2] = {0};
  data[0] = 0xff & (id>>8);
  data[1] = 0xff&id;

  
  cout << "number: " << id << hex << " 0x" << id << dec << endl;
  cout << "bytes: " << hex << static_cast<int>(data[0]) << static_cast<int>(data[1]) << dec << endl;
  //  cout << "crc(" << sid << ") = " << hex << static_cast<int>(SMF3000_Crc(data, 2)) << endl;
  cout << "crc(" << sid << ") = " << hex << static_cast<int>(crc(data, 2)) << endl;
  return 0;
}
 
