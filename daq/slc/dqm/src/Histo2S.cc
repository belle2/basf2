#include "daq/slc/dqm/Histo2S.h"

#include "daq/slc/dqm/ShortArray.h"

using namespace Belle2;

Histo2S::Histo2S() : Histo2()
{
  m_data = new ShortArray();
}

Histo2S::Histo2S(const Histo2S& h) : Histo2(h)
{
  if (h.m_data != NULL) m_data = new ShortArray(*h.m_data);
  else m_data = new ShortArray();
}

Histo2S::Histo2S(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  m_data = new ShortArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2S::getDataType() const throw()
{
  return std::string("H2S");
}
