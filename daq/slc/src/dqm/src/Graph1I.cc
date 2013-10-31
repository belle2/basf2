#include "Graph1I.h"

#include "IntArray.h"

using namespace Belle2;

Graph1I::Graph1I() : Graph1()
{
  _data = new IntArray(0);
}

Graph1I::Graph1I(const Graph1I& h) : Graph1(h)
{
  _data = new IntArray(*_data);
}

Graph1I::Graph1I(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  _data = new IntArray(nbinx * 2);
}

Graph1I::~Graph1I() throw() {}

std::string Graph1I::getDataType() const throw()
{
  return std::string("g1I");
}

