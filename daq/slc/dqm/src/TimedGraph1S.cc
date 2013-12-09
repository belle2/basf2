#include "daq/slc/dqm/TimedGraph1S.h"

#include "daq/slc/dqm/ShortArray.h"

using namespace Belle2;

TimedGraph1S::TimedGraph1S() throw() : TimedGraph1()
{
  _data = new ShortArray(0);
}

TimedGraph1S::TimedGraph1S(const TimedGraph1S& h) throw() : TimedGraph1(h)
{
  _data = new ShortArray(h._data->size());
}

TimedGraph1S::TimedGraph1S(const std::string& name, const std::string& title,
                           int nbinx, double xmin, double xmax)
throw() : TimedGraph1(name, title, nbinx, xmin, xmax)
{
  _data = new ShortArray(nbinx);
  _time_v.resize(nbinx);
}

TimedGraph1S::~TimedGraph1S() throw() {}

std::string TimedGraph1S::getDataType() const throw()
{
  return std::string("TGS");
}

