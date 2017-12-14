#include "daq/slc/dqm/Graph1I.h"

#include "daq/slc/dqm/IntArray.h"

using namespace Belle2;

Graph1I::Graph1I() : Graph1()
{
  m_data = new IntArray(0);
}

Graph1I::Graph1I(const Graph1I& h) : Graph1(h)
{
  m_data = new IntArray(*h.m_data);
}

Graph1I::Graph1I(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax, double ymin, double ymax)
  : Graph1(name, title, nbinx, xmin, xmax, ymin, ymax)
{
  m_data = new IntArray(nbinx * 2);
}

Graph1I::~Graph1I() throw() {}

std::string Graph1I::getDataType() const throw()
{
  return std::string("g1I");
}

