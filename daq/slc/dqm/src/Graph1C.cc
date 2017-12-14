#include "daq/slc/dqm/Graph1C.h"

#include "daq/slc/dqm/CharArray.h"

using namespace Belle2;

Graph1C::Graph1C() : Graph1()
{
  m_data = new CharArray(0);
}

Graph1C::Graph1C(const Graph1C& h) : Graph1(h)
{
  m_data = new CharArray(*h.m_data);
}

Graph1C::Graph1C(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  m_data = new CharArray(nbinx * 2);
}

Graph1C::~Graph1C() throw() {}

std::string Graph1C::getDataType() const throw()
{
  return std::string("g1C");
}

