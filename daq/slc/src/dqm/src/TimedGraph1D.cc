#include "dqm/TimedGraph1D.h"

#include "dqm/DoubleArray.h"

using namespace Belle2;

TimedGraph1D::TimedGraph1D() throw() : TimedGraph1()
{
  _data = new DoubleArray(0);
}

TimedGraph1D::TimedGraph1D(const TimedGraph1D& h) throw() : TimedGraph1(h)
{
  _data = new DoubleArray(h._data->size());
}

TimedGraph1D::TimedGraph1D(const std::string& name, const std::string& title,
                           int nbinx, double xmin, double xmax)
throw() : TimedGraph1(name, title, nbinx, xmin, xmax)
{
  _data = new DoubleArray(nbinx);
  _time_v.resize(nbinx);
}

TimedGraph1D::~TimedGraph1D() throw() {}

std::string TimedGraph1D::getDataType() const throw()
{
  return std::string("TGD");
}

