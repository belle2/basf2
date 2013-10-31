#include "Histo2S.h"

#include "ShortArray.h"

using namespace Belle2;

Histo2S::Histo2S() : Histo2()
{
  _data = new ShortArray();
}

Histo2S::Histo2S(const Histo2S& h) : Histo2(h)
{
  if (h._data != NULL) _data = new ShortArray(*h._data);
  else _data = new ShortArray();
}

Histo2S::Histo2S(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  _data = new ShortArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2S::getDataType() const throw()
{
  return std::string("H2S");
}
