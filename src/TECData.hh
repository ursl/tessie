#ifndef TECDATA_HH
#define TECDATA_HH

#include <iostream>
#include <map>
#include "TECRegister.hh"

// ----------------------------------------------------------------------
struct TECData {
  int getIdx(std::string regname) {
    // FIXME: This is wrong and always returns 0!
    int idx = reg.find(regname)->second.idx;
    // FIXME: Therefore replace with for loop
    for (std::map<std::string, TECRegister>::iterator it = reg.begin(); it != reg.end(); ++it) {
        if (std::string::npos != it->first.find(regname)) {
            if (0) std::cout << "regname " << regname << " -> idx = " << idx
                             << " second = " << reg.find(regname)->second.name
                             << std::endl;
            return idx;
          }
      }
    return -1;
  }
  void print() {
    for (std::map<std::string, TECRegister>::iterator it = reg.begin(); it != reg.end(); ++it) {
        std::cout << it->first << " idx: " << it->second.idx
                  << " value: " << it->second.value
                  << " type: " << it->second.type
                  << std::endl;
      }
  }
  std::map<std::string, TECRegister> reg;
};

#endif // TECDATA_HH
