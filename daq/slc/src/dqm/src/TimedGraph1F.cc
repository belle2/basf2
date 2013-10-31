#include "TimedGraph1F.h"

#include "FloatArray.h"

using namespace Belle2;

TimedGraph1F::TimedGraph1F() throw() : TimedGraph1()
{
  _data = new FloatArray(0);
}

TimedGraph1F::TimedGraph1F(const TimedGraph1F& h) throw() : TimedGraph1(h)
{
  _data = new FloatArray(h._data->size());
}

TimedGraph1F::TimedGraph1F(const std::string& name, const std::string& title,
                           int nbinx, double xmin, double xmax)
throw() : TimedGraph1(name, title, nbinx, xmin, xmax)
{
  _data = new FloatArray(nbinx);
  _time_v.resize(nbinx);
}

TimedGraph1F::~TimedGraph1F() throw() {}

std::string TimedGraph1F::getDataType() const throw()
{
  return std::string("TGF");
}

