#include "daq/slc/dqm/TimedGraph1I.h"

#include "daq/slc/dqm/IntArray.h"

using namespace Belle2;

TimedGraph1I::TimedGraph1I() throw() : TimedGraph1()
{
  _data = new IntArray(0);
}

TimedGraph1I::TimedGraph1I(const TimedGraph1I& h) throw() : TimedGraph1(h)
{
  _data = new IntArray(h._data->size());
}

TimedGraph1I::TimedGraph1I(const std::string& name, const std::string& title,
                           int nbinx, double xmin, double xmax)
throw() : TimedGraph1(name, title, nbinx, xmin, xmax)
{
  _data = new IntArray(nbinx);
  _time_v.resize(nbinx);
}

TimedGraph1I::~TimedGraph1I() throw() {}

std::string TimedGraph1I::getDataType() const throw()
{
  return std::string("TGI");
}

