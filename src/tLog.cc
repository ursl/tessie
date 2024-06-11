#include <cstdio>
#include <iostream>
#include <sstream>

#include "tLog.hh"

using namespace std;

// ----------------------------------------------------------------------
tLog::tLog(string fname): fLevel(INFO), fFileName(fname) {
  fFile.open(fFileName, ios_base::app);
  int filesize = fFile.tellp();
  if (filesize > 1000) {
    string bacname = fFileName + string(".") + timeStamp(true);

    string from("/"), to("");
    size_t start_pos = 0;
    while((start_pos = bacname.find(from, start_pos)) != string::npos) {
      bacname.replace(start_pos, from.length(), to);
      start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }

    cout << "filesize = " << filesize << " too large, creating new logfile" << endl;
    fFile << "filesize = " << filesize << " too large, backup to " << bacname
          << " and creating new logfile"
          << endl;
    fFile.close();

    int result = rename(fFileName.c_str(), bacname.c_str());
    cout << "renamed " << fFileName << " to " << bacname << " with result = " << result << endl;

    fFile.open(fFileName, ios_base::app);
    fFile << "filesize = " << filesize << " too large, backed up to " << bacname
          << " and starting new logfile"
          << endl;
  }
}

// ----------------------------------------------------------------------
tLog:: ~tLog() {
  fFile.close();
}

// ----------------------------------------------------------------------
string tLog::print(tLogLevel level, std::string print) {
  string slevel = toString(level);
  stringstream output;
  output << timeStamp() << " "
	 << std::setfill(' ') << std::setw(7) << slevel << " "
	 << print;
  string sout = output.str();
  fFile << sout << endl;
  emit signalText(QString::fromStdString(sout));
  return sout;
}


// ----------------------------------------------------------------------
void tLog::operator()(tLogLevel level, std::string print) {
  string slevel = toString(level);
  stringstream output;
  output << timeStamp() << " "
	 << std::setfill(' ') << std::setw(7) << slevel << " "
	 << print;
  stringstream teTimeStamp;
  teTimeStamp << timeStamp(false)  << " "
              << std::setfill(' ') << std::setw(7) << slevel << " "
              << print;
  string sout = output.str();
  fFile << sout << endl;
  if (level <= fLevel) {
    string teout = teTimeStamp.str();
    emit signalText(QString::fromStdString(teout));
  }
  cout << sout << endl;
}



// ----------------------------------------------------------------------
tLogLevel tLog::getLevel() {
  return fLevel;
}


// ----------------------------------------------------------------------
void tLog::setLevel(string level) {
  fLevel = fromString(level);
}


// ----------------------------------------------------------------------
std::string tLog::toString(tLogLevel level) {
  static const char* const buffer[] = {"ERROR", "WARNING", "INFO", "DEBUG", "ALL"};
  return buffer[level];
}

// ----------------------------------------------------------------------
tLogLevel tLog::fromString(const std::string& level) {
  if (level == "DEBUG")
    return DEBUG;
  if (level == "INFO")
    return INFO;
  if (level == "WARNING")
    return WARNING;
  if (level == "ERROR")
    return ERROR;
  if (level == "ALL")
    return ALL;
  // give up
  return ALL;
}



// ----------------------------------------------------------------------
string tLog::timeStamp(bool filestamp) {
  char buffer[11];
  time_t t;
  time(&t);
  tm r;
  strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);

  tm *ltm = localtime(&t);
  int year  = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day   = ltm->tm_mday;
  int hour  = ltm->tm_hour;
  int min   = ltm->tm_min;
  int sec   = ltm->tm_sec;
  std::stringstream result;
  if (filestamp) {
    result << year << "/"
           << std::setfill('0') << std::setw(2) << month << "/"
           << std::setfill('0') << std::setw(2) << day << "_";
  }
  result << std::setfill('0') << std::setw(2) << hour << ":"
         << std::setfill('0') << std::setw(2) << min << ":"
         << std::setfill('0') << std::setw(2) << sec << "."
         << std::setfill('0') << std::setw(3) << ((long)tv.tv_usec)/1000;
  return result.str();
}


// ----------------------------------------------------------------------
string tLog::shortTimeStamp() {
  char buffer[11];
  time_t t;
  time(&t);
  tm r;
  strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);

  tm *ltm = localtime(&t);
  int year  = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day   = ltm->tm_mday;
  int hour  = ltm->tm_hour;
  int min   = ltm->tm_min;
  int sec   = ltm->tm_sec;
  std::stringstream result;
  result << year << "/"
         << std::setfill('0') << std::setw(2) << month << "/"
         << std::setfill('0') << std::setw(2) << day << " ";
  result << std::setfill('0') << std::setw(2) << hour << ":"
         << std::setfill('0') << std::setw(2) << min << ":"
         << std::setfill('0') << std::setw(2) << sec ;
  return result.str();
}
