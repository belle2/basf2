#include "dqm/Histo1I.h"

#include "dqm/IntArray.h"

using namespace Belle2;

Histo1I::Histo1I() throw() : Histo1()
{
  _data = new IntArray(0);
}

Histo1I::Histo1I(const Histo1I& h) throw() : Histo1(h)
{
  _data = new IntArray(h._data->size());
}

Histo1I::Histo1I(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  _data = new IntArray(nbinx + 2);
}

Histo1I::~Histo1I() throw() {}

std::string Histo1I::getDataType() const throw()
{
  return std::string("H1I");
}
