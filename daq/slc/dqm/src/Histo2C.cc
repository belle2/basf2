#include "daq/slc/dqm/Histo2C.h"

#include "daq/slc/dqm/CharArray.h"

using namespace Belle2;

Histo2C::Histo2C() : Histo2()
{
  m_data = new CharArray();
}

Histo2C::Histo2C(const Histo2C& h) : Histo2(h)
{
  if (h.m_data != NULL) m_data = new CharArray(*h.m_data);
  else m_data = new CharArray();
}

Histo2C::Histo2C(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  m_data = new CharArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2C::getDataType() const throw()
{
  return std::string("H2C");
}
