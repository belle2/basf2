#include "TimedGraph1C.h"

#include "CharArray.h"

using namespace Belle2;

TimedGraph1C::TimedGraph1C() throw() : TimedGraph1()
{
  _data = new CharArray(0);
}

TimedGraph1C::TimedGraph1C(const TimedGraph1C& h) throw() : TimedGraph1(h)
{
  _data = new CharArray(h._data->size());
}

TimedGraph1C::TimedGraph1C(const std::string& name, const std::string& title,
                           int nbinx, double xmin, double xmax)
throw() : TimedGraph1(name, title, nbinx, xmin, xmax)
{
  _data = new CharArray(nbinx);
  _time_v.resize(nbinx);
}

TimedGraph1C::~TimedGraph1C() throw() {}

std::string TimedGraph1C::getDataType() const throw()
{
  return std::string("TGC");
}

