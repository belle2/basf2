#include "Graph1F.hh"

#include "FloatArray.hh"

using namespace B2DQM;

Graph1F::Graph1F() : Graph1()
{
  _data = new FloatArray(0);
}

Graph1F::Graph1F(const Graph1F& h) : Graph1(h)
{
  _data = new FloatArray(*_data);
}

Graph1F::Graph1F(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  _data = new FloatArray(nbinx * 2);
}

Graph1F::~Graph1F() throw() {}

std::string Graph1F::getDataType() const throw()
{
  return std::string("g1F");
}

