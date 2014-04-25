#include "daq/slc/dqm/Histo1I.h"

#include "daq/slc/dqm/IntArray.h"

using namespace Belle2;

Histo1I::Histo1I() throw() : Histo1()
{
  m_data = new IntArray(0);
}

Histo1I::Histo1I(const Histo1I& h) throw() : Histo1(h)
{
  m_data = new IntArray(h.m_data->size());
}

Histo1I::Histo1I(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  m_data = new IntArray(nbinx + 2);
}

Histo1I::~Histo1I() throw() {}

std::string Histo1I::getDataType() const throw()
{
  return std::string("H1I");
}
