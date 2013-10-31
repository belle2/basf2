#include "Graph1C.h"

#include "CharArray.h"

using namespace Belle2;

Graph1C::Graph1C() : Graph1()
{
  _data = new CharArray(0);
}

Graph1C::Graph1C(const Graph1C& h) : Graph1(h)
{
  _data = new CharArray(*_data);
}

Graph1C::Graph1C(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  _data = new CharArray(nbinx * 2);
}

Graph1C::~Graph1C() throw() {}

std::string Graph1C::getDataType() const throw()
{
  return std::string("g1C");
}

