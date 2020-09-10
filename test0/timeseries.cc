#include "timeseries.hh"

using namespace std;
using namespace QtCharts;

// ----------------------------------------------------------------------
timeseries::timeseries(string name, int npoints): fToday(new QLineSeries()), fHistory(), fName(name), fNpointsOnAxis(npoints) {

}


// ----------------------------------------------------------------------
timeseries::~timeseries() {
  vector<QtCharts::QLineSeries*>::iterator end = fHistory.end();
  for (vector<QtCharts::QLineSeries*>::iterator il = fHistory.begin(); il != end; ++il) {
    // do something
  }
  fHistory.clear();
}


// ----------------------------------------------------------------------
void timeseries::add(double x, double y) {
  if (fToday->count() < fNpointsOnAxis) {
    fToday->append(x, y);
  } else {
    fHistory.push_back(fToday);
    string pn = fNames.back();
    int ipn = ::atoi(pn.c_str());
    string newname = to_string(ipn++);
    fNames.push_back(newname);
    fToday = new QLineSeries();
    fToday->append(x, y);
  }
}

// ----------------------------------------------------------------------
void timeseries::setName(int idx, std::string name) {
  if (fNames.size() > idx) {
    fNames[idx] = name;
  }
}


// ----------------------------------------------------------------------
pair<QtCharts::QLineSeries*, string> timeseries::get(int idx) {
  if (idx < static_cast<int>(fHistory.size())) {
    return make_pair(fHistory[idx], fNames[idx]);
  } else {
    return make_pair(fHistory[fHistory.size()-1], fNames[fHistory.size()-1]);
  }
}
