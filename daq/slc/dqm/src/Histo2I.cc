#include "daq/slc/dqm/Histo2I.h"

#include "daq/slc/dqm/IntArray.h"

using namespace Belle2;

Histo2I::Histo2I() : Histo2()
{
  m_data = new IntArray();
}

Histo2I::Histo2I(const Histo2I& h) : Histo2(h)
{
  if (h.m_data != NULL) m_data = new IntArray(*h.m_data);
  else m_data = new IntArray();
}

Histo2I::Histo2I(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  m_data = new IntArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2I::getDataType() const throw()
{
  return std::string("H2I");
}
