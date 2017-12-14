#include "daq/slc/dqm/Histo1C.h"

#include "daq/slc/dqm/CharArray.h"

using namespace Belle2;

Histo1C::Histo1C() throw() : Histo1()
{
  m_data = new CharArray(0);
}

Histo1C::Histo1C(const Histo1C& h) throw() : Histo1(h)
{
  m_data = new CharArray(h.m_data->size());
}

Histo1C::Histo1C(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  m_data = new CharArray(nbinx + 2);
}

Histo1C::~Histo1C() throw()
{
}

std::string Histo1C::getDataType() const throw()
{
  return std::string("H1C");
}
