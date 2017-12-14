#include "daq/slc/dqm/Histo2D.h"

#include "daq/slc/dqm/DoubleArray.h"

using namespace Belle2;

Histo2D::Histo2D() : Histo2()
{
  m_data = new DoubleArray();
}

Histo2D::Histo2D(const Histo2D& h) : Histo2(h)
{
  if (h.m_data != NULL) m_data = new DoubleArray(*h.m_data);
  else m_data = new DoubleArray();
}

Histo2D::Histo2D(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  m_data = new DoubleArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2D::getDataType() const throw()
{
  return std::string("H2D");
}
