#include <iostream>

#include "trpc.h"

using namespace std;

// ----------------------------------------------------------------------
int main (int argc, char **argv) {
  cout << "hello before server start" << endl;
  startServer();
  cout << "hello after server start" << endl;
  return 0;
}
