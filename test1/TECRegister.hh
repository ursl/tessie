#ifndef TECREGISTER_HH
#define TECREGISTER_HH

#include <iostream>

// ----------------------------------------------------------------------
struct TECRegister {
  float       value;
  std::string name;
  uint32_t    idx;
  uint32_t    type; // 0 = unset, 1 = W/R, 2 = R, 3 = C
};

#endif // TECREGISTER_HH
