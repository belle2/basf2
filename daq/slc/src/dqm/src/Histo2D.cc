#include "dqm/Histo2D.h"

#include "dqm/DoubleArray.h"

using namespace Belle2;

Histo2D::Histo2D() : Histo2()
{
  _data = new DoubleArray();
}

Histo2D::Histo2D(const Histo2D& h) : Histo2(h)
{
  if (h._data != NULL) _data = new DoubleArray(*h._data);
  else _data = new DoubleArray();
}

Histo2D::Histo2D(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  _data = new DoubleArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2D::getDataType() const throw()
{
  return std::string("H2D");
}
