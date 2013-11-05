#include "dqm/Histo2I.h"

#include "dqm/IntArray.h"

using namespace Belle2;

Histo2I::Histo2I() : Histo2()
{
  _data = new IntArray();
}

Histo2I::Histo2I(const Histo2I& h) : Histo2(h)
{
  if (h._data != NULL) _data = new IntArray(*h._data);
  else _data = new IntArray();
}

Histo2I::Histo2I(const std::string& name, const std::string& title,
                 int nbinx, double xmin, double xmax,
                 int nbiny, double ymin, double ymax)
  : Histo2(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax)
{
  _data = new IntArray((nbinx + 2) * (nbiny + 2));
}

std::string Histo2I::getDataType() const throw()
{
  return std::string("H2I");
}
