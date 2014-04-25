#include "daq/slc/dqm/Histo1D.h"

#include "daq/slc/dqm/DoubleArray.h"

using namespace Belle2;

Histo1D::Histo1D() throw() : Histo1()
{
  m_data = new DoubleArray(0);
}

Histo1D::Histo1D(const Histo1D& h) throw() : Histo1(h)
{
  m_data = new DoubleArray(h.m_data->size());
}

Histo1D::Histo1D(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  m_data = new DoubleArray(nbinx + 2);
}

Histo1D::~Histo1D() throw() {}

std::string Histo1D::getDataType() const throw()
{
  return std::string("H1D");
}
