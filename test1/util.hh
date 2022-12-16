#ifndef UTIL_HH
#define UTIL_HH

#include <string>
#include <vector>

void replaceAll(std::string &s, const std::string &from, const std::string &to);
std::vector<std::string>  split(const std::string &s, char delim);
void split(const std::string &s, char delim, std::vector<std::string> &elems); // helper function for above

#endif // UTIL_HH
