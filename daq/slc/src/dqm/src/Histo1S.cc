#include "Histo1S.h"

#include "ShortArray.h"

using namespace Belle2;

Histo1S::Histo1S() throw() : Histo1()
{
  _data = new ShortArray(0);
}

Histo1S::Histo1S(const Histo1S& h) throw() : Histo1(h)
{
  _data = new ShortArray(h._data->size());
}

Histo1S::Histo1S(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  _data = new ShortArray(nbinx + 2);
}

Histo1S::~Histo1S() throw() {}

std::string Histo1S::getDataType() const throw()
{
  return std::string("H1S");
}
