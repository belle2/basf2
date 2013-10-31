#include "Histo1C.h"

#include "CharArray.h"

using namespace Belle2;

Histo1C::Histo1C() throw() : Histo1()
{
  _data = new CharArray(0);
}

Histo1C::Histo1C(const Histo1C& h) throw() : Histo1(h)
{
  _data = new CharArray(h._data->size());
}

Histo1C::Histo1C(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  _data = new CharArray(nbinx + 2);
}

Histo1C::~Histo1C() throw()
{
}

std::string Histo1C::getDataType() const throw()
{
  return std::string("H1C");
}
