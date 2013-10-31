#include "Histo1F.h"

#include "FloatArray.h"

using namespace Belle2;

Histo1F::Histo1F() throw() : Histo1()
{
  _data = new FloatArray(0);
}

Histo1F::Histo1F(const Histo1F& h) throw() : Histo1(h)
{
  _data = new FloatArray(h._data->size());
}

Histo1F::Histo1F(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  _data = new FloatArray(nbinx + 2);
}

Histo1F::~Histo1F() throw() {}

std::string Histo1F::getDataType() const throw()
{
  return std::string("H1F");
}
