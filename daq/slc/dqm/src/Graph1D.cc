#include "daq/slc/dqm/Graph1D.h"

#include "daq/slc/dqm/DoubleArray.h"

using namespace Belle2;

Graph1D::Graph1D() : Graph1()
{
  m_data = new DoubleArray(0);
}

Graph1D::Graph1D(const Graph1D& h) : Graph1(h)
{
  m_data = new DoubleArray(*h.m_data);
}

Graph1D::Graph1D(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  m_data = new DoubleArray(nbinx * 2);
}

Graph1D::~Graph1D() throw() {}

std::string Graph1D::getDataType() const throw()
{
  return std::string("g1D");
}

