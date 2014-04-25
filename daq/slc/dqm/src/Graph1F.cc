#include "daq/slc/dqm/Graph1F.h"

#include "daq/slc/dqm/FloatArray.h"

using namespace Belle2;

Graph1F::Graph1F() : Graph1()
{
  m_data = new FloatArray(0);
}

Graph1F::Graph1F(const Graph1F& h) : Graph1(h)
{
  m_data = new FloatArray(*h.m_data);
}

Graph1F::Graph1F(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  m_data = new FloatArray(nbinx * 2);
}

Graph1F::~Graph1F() throw() {}

std::string Graph1F::getDataType() const throw()
{
  return std::string("g1F");
}

