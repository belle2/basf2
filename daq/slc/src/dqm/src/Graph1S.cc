#include "Graph1S.h"

#include "ShortArray.h"

using namespace Belle2;

Graph1S::Graph1S() : Graph1()
{
  _data = new ShortArray(0);
}

Graph1S::Graph1S(const Graph1S& h) : Graph1(h)
{
  _data = new ShortArray(*_data);
}

Graph1S::Graph1S(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  _data = new ShortArray(nbinx * 2);
}

Graph1S::~Graph1S() throw() {}

std::string Graph1S::getDataType() const throw()
{
  return std::string("g1S");
}

