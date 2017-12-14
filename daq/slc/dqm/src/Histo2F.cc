#include "daq/slc/dqm/Histo2F.h"

#include "daq/slc/dqm/FloatArray.h"

using namespace Belle2;

Histo2F::Histo2F() : Histo2()
{
  m_data = new FloatArray();
}

Histo2F::Histo2F(const Histo2F& h) : Histo2(h)
{
  if (h.m_data != NULL) m_data = new FloatArray(*h.m_data);
  else m_data = new FloatArray();
}

Histo2F::Histo2F(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  m_data = new FloatArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2F::getDataType() const throw()
{
  return std::string("H2F");
}
