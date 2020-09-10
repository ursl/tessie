#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <fstream>
#include <string>
#include <cstdio>
#include <iomanip>

#include <QtCore/QDateTime>
#include <QtCharts/QLineSeries>

class timeseries {
  timeseries(std::string name = "nada", int npoints = 8640);
  ~timeseries();

  // -- add a new point
  void add(double, double);
  // -- allow setting name at idx
  void setName(int idx, std::string name);
  // -- return element idx from fHistory and fNames
  std::pair<QtCharts::QLineSeries*, std::string> get(int idx);

  // -- the data for today
  QtCharts::QLineSeries *fToday;
  // -- cached QLineSeries return to previous axes
  std::vector<QtCharts::QLineSeries*> fHistory;
  std::vector<std::string> fNames;

  std::string fName;
  // -- this is needed to know when to migrate today's data to the history
  int fNpointsOnAxis;

};

#endif
